/*
 * Copyright (c) 2006-2020, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-03-19     CCRFIDZY       the first version
 */
#include <rtthread.h>
#include "App_Eth.h"
#include "APP_Core.h"
#include "drv_flash.h"
#include "netdev.h"

extern const unsigned char user_dat[0x4000];
extern struct rt_event ETH_EVENT;

static int ETH_Sock;  //sock句柄  
static struct sockaddr_in Eth_client;
static struct sockaddr_in Eth_client2;

/* 定时器的控制块 */
static rt_timer_t timer3;

static void ETHUDP_Link(void);
static void ETH_GetDHCP_IP(void);

static uint8_t ETH_PackNumber=0; //消息流水号
static uint16_t heart_cnt=0;			//心跳计时

/* 定时器 3 超时函数 */
static void timeout3(void *parameter)
{
	if(ETH_Sock != 0)
	{
		//前100s 每10s发上报帧，直到应答
		if(heart_cnt<10)
		{
			if(user_dat[DHCP_Add] == 1) //动态获取IP的时候
			{
				ETH_GetDHCP_IP();
			}
			heart_cnt++;
			rt_event_send(&ETH_EVENT,EVENT_UPLOAD);
		}
		else if(heart_cnt >= 40) //300秒 5分钟一次心跳
		{
			heart_cnt = 10;
			rt_event_send(&ETH_EVENT,EVENT_HEART);
		}
		else
			heart_cnt++;
	}
	else
	{
			ETHUDP_Link();
	}
}
void APP_UDPETHCOMTx_entry(void* parameter)
{
	uint8_t send_databuf[64];
	uint8_t len;
	uint32_t CMD;
	char tag_data[128];
	if (ETH_Sock != NULL)
	{
		rt_timer_start(timer3);
	}
	while (1)
	{
		if (rt_event_recv(&ETH_EVENT, EVENT_ALL,
                      RT_EVENT_FLAG_OR | RT_EVENT_FLAG_CLEAR,
                      RT_WAITING_FOREVER, &CMD) == RT_EOK)
		{
			switch(CMD)
			{
				case EVENT_HEART:
				{
					len = uc_HeartDataPackage(send_databuf,ETH_PackNumber++);
				}break;
				case EVENT_UPLOAD:
				{
					len = uc_UploadDataPackage(send_databuf,ETH_PackNumber++);
				}break;
				case EVENT_TAG_UPLOAD:
				{
					len = 0;
					Dequeue(ETH_SEND_QUEUE, (uint8_t *)tag_data);
					len = uc_TAGNormalDataPackage(send_databuf,(uint8_t *)tag_data,ETH_PackNumber++);
				}break;
				default:len=0;break;
			}
			if(len != 0)
			{
					sendto(ETH_Sock, send_databuf, len, 0,(struct sockaddr *)&Eth_client, sizeof(struct sockaddr));
					if(user_dat[SIPx11_Add] == user_dat[SIP1_Add])
						sendto(ETH_Sock, send_databuf, len, 0,(struct sockaddr *)&Eth_client2, sizeof(struct sockaddr));
					memset(send_databuf,0,len);
			}
		}
	}
}


void APP_UDPETHCOMRx_entry(void* parameter)
{
	uint8_t res=0;
	char recv_data[64];
	socklen_t addr_len;
	uint8_t state[4];
	uint8_t tag_id[4];
	int ret;
	struct sockaddr_in client_addr;
	addr_len = sizeof(struct sockaddr);

	while (1)
	{
			ret = recvfrom(ETH_Sock, recv_data, 63, 0,(struct sockaddr *)&client_addr, &addr_len); 

			//只接收来自目的主机的数据
			//解析
			res = uc_RDMsgPackage((uint8_t *)recv_data,ret,(uint8_t *)state,tag_id);
			switch (res)
			{
				case HEART://应答心跳
				{
					heart_cnt = 10;
				}break;
				case WRCONFIG:
				{
					ret = uc_ReSetPackage((uint8_t *)recv_data,ETH_PackNumber++,state[0]);
					sendto(ETH_Sock, recv_data, ret, 0,(struct sockaddr *)&client_addr, sizeof(struct sockaddr));
					rt_event_send(&Core_EVENT,EVENT_REBOOT);
				}break;
				case RDCONFIG:
				{
					ret = uc_ReReadStatPackage((uint8_t *)recv_data,ETH_PackNumber++,state);
					sendto(ETH_Sock, recv_data, ret, 0,(struct sockaddr *)&client_addr, sizeof(struct sockaddr));
				}break;
				case TAG_WRCONFIG:
				{
					ret = uc_ReSetTagPackage((uint8_t *)recv_data,tag_id,ETH_PackNumber++,state[0]);
					sendto(ETH_Sock, recv_data, ret, 0,(struct sockaddr *)&client_addr, sizeof(struct sockaddr));
				}break;
				case TAG_RDCONFIG:
				{
					ret = uc_ReReadTagStatPackage((uint8_t *)recv_data,tag_id,ETH_PackNumber++,state);
					sendto(ETH_Sock, recv_data, ret, 0,(struct sockaddr *)&client_addr, sizeof(struct sockaddr));
				}break;
				default:break;
			}
	}
}

void thread_ETH_init(void)
{
	rt_thread_t RT_ETHCOMRx = RT_NULL;
	rt_thread_t RT_ETHCOMTx = RT_NULL;
		
	ETHUDP_Link();	
	/* 创建定时器 3  周期定时器 */
	timer3 = rt_timer_create("timer3", timeout3,RT_NULL, 10000,RT_TIMER_FLAG_PERIODIC);
	/* 初始化一个 mailbox */
	rt_event_init(&ETH_EVENT, "event", RT_IPC_FLAG_FIFO);			
	RT_ETHCOMTx = rt_thread_create("ETHTx",APP_UDPETHCOMTx_entry,RT_NULL,ETHCOM_TXSTACK_SIZE,ETHCOM_TXPRIORITY,ETHCOM_TIMESLICE);	
	RT_ETHCOMRx = rt_thread_create("ETHRx",APP_UDPETHCOMRx_entry,RT_NULL,ETHCOM_RXSTACK_SIZE,ETHCOM_RXPRIORITY,ETHCOM_TIMESLICE);
	if (RT_ETHCOMRx != RT_NULL)
	{
		rt_thread_startup(RT_ETHCOMRx);
		rt_thread_startup(RT_ETHCOMTx);
	}
}


static void ETHUDP_Link(void)
{
	if ((ETH_Sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
	{
			rt_kprintf("Socket error\n"); 
	}
	else
	{
		struct sockaddr_in server_addr;			
		/* 初始化本地IP地址 */
		uint16_t ETH_LocalPort;
		uint16_t ETH_SerPort;
		uint32_t ETH_SerIP;
		ETH_LocalPort = user_dat[CPort1_Add];
		ETH_LocalPort = (ETH_LocalPort<<8) + user_dat[CPort2_Add];

		server_addr.sin_family = AF_INET;
		server_addr.sin_port = htons(ETH_LocalPort);
		server_addr.sin_addr.s_addr = IPADDR_ANY;
		rt_memset(&(server_addr.sin_zero), 0, sizeof(server_addr.sin_zero));

		//配置服务器IP和端口 默认为广播
		ETH_SerPort = user_dat[SPort1_Add];
		ETH_SerPort = (ETH_SerPort<<8) + user_dat[SPort2_Add];
		ETH_SerIP = user_dat[SIP4_Add];
		ETH_SerIP = (ETH_SerIP<<8) + user_dat[SIP3_Add];
		ETH_SerIP = (ETH_SerIP<<8) + user_dat[SIP2_Add];
		ETH_SerIP = (ETH_SerIP<<8) + user_dat[SIP1_Add];
		Eth_client.sin_family = AF_INET;
		Eth_client.sin_port = htons(ETH_SerPort);
		Eth_client.sin_addr.s_addr = ETH_SerIP;
		rt_memset(&(Eth_client.sin_zero), 0, sizeof(Eth_client.sin_zero));
			
		/* 绑定socket到服务端地址 */
		if (bind(ETH_Sock, (struct sockaddr *)&server_addr,sizeof(struct sockaddr)) == -1)
		{
			/* 绑定地址失败 */
			closesocket(ETH_Sock);
			ETH_Sock = 0;
			rt_kprintf("Bind error\n");
		}
		else
		{
			int flag=1;
			setsockopt(ETH_Sock, SOL_SOCKET, SO_BROADCAST, &flag, sizeof(flag));
			struct netdev *netdev = RT_NULL;
			netdev = netdev_get_by_name("e0");
			if(netdev != RT_NULL)
			{
				if(user_dat[DHCP_Add] == 0)//DHCP关闭
				{
					char ip[20],mask[20],gateway[20];
					netdev_dhcp_enabled(netdev,RT_FALSE);
					sprintf(ip,"%d.%d.%d.%d",user_dat[CIP1_Add],user_dat[CIP2_Add],user_dat[CIP3_Add],user_dat[CIP4_Add]);
					sprintf(mask,"%d.%d.%d.%d",user_dat[Mask1_Add],user_dat[Mask2_Add],user_dat[Mask3_Add],user_dat[Mask4_Add]);
					sprintf(gateway,"%d.%d.%d.%d",user_dat[Gateway1_Add],user_dat[Gateway2_Add],user_dat[Gateway3_Add],user_dat[Gateway4_Add]);
					netdev_set_if(netdev->name,ip,gateway,mask);
					netdev = netdev_get_by_name("e0");
				}
			}
		}
		
		if(user_dat[SIPx11_Add] == user_dat[SIP1_Add]) //同一网段还有ip组播
		{
			uint16_t ETH_SerPort;
			uint32_t ETH_SerIP;
			//配置服务器IP和端口 默认为广播
			ETH_SerPort = user_dat[SPortx11_Add];
			ETH_SerPort = (ETH_SerPort<<8) + user_dat[SPortx12_Add];
			ETH_SerIP = user_dat[SIPx14_Add];
			ETH_SerIP = (ETH_SerIP<<8) + user_dat[SIPx13_Add];
			ETH_SerIP = (ETH_SerIP<<8) + user_dat[SIPx12_Add];
			ETH_SerIP = (ETH_SerIP<<8) + user_dat[SIPx11_Add];
			Eth_client2.sin_family = AF_INET;
			Eth_client2.sin_port = htons(ETH_SerPort);
			Eth_client2.sin_addr.s_addr = ETH_SerIP;
			rt_memset(&(Eth_client2.sin_zero), 0, sizeof(Eth_client2.sin_zero));
		}
	}
}
	
static void ETH_GetDHCP_IP(void)
{
	uint8_t FLASH_BUFF[100];
	struct netdev *netdev = RT_NULL;

	netdev = netdev_get_by_name("e0");
	if(netdev->ip_addr.addr != 0)
	{
		stm32_flash_read(Memery_Start_Address,FLASH_BUFF,100);
		FLASH_BUFF[CIP1_Add] = netdev->ip_addr.addr;
		FLASH_BUFF[CIP2_Add] = netdev->ip_addr.addr>>8;
		FLASH_BUFF[CIP3_Add] = netdev->ip_addr.addr>>16;
		FLASH_BUFF[CIP4_Add] = netdev->ip_addr.addr>>24;
		stm32_flash_erase(Memery_Start_Address,100);
		stm32_flash_write(Memery_Start_Address,FLASH_BUFF,100);
	}
}



