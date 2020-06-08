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
struct rt_event ETH_EVENT;
static int ETH_Sock,ETH_Sock2;  //sock句柄  


/* 定时器的控制块 */
static rt_timer_t timer3;

int ETHCOM_TCPLink(uint32_t IP_s, int port_s);
static void ETH_GetDHCP_IP(void);

static uint8_t ETH_PackNumber=0; //消息流水号
static uint16_t heart_cnt=0;			//心跳计时

/* 定时器 3 超时函数 */
static void timeout3(void *parameter)
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


void APP_TCPETHCOMTx_entry(void* parameter)
{
		int ret;
		size_t len;
		uint8_t send_databuf[64];
		uint32_t CMD;
		char tag_data[128];
		while (1)
    {
			if(ETH_Sock != 0)
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
							if(tag_data != RT_NULL)
							{
								Dequeue(ETH_SEND_QUEUE, (uint8_t *)tag_data);
								len = uc_TAGNormalDataPackage(send_databuf,(uint8_t *)tag_data,ETH_PackNumber++);
							}
						}break;
						default:len=0;break;
					}
					
					ret = send(ETH_Sock, send_databuf,len, 0);
					if (ret < 0)
					{
							/* 发送失败 关闭连接 */
							closesocket(ETH_Sock);
							ETH_Sock = 0;
					}
					if(ETH_Sock2 != 0)
					{
						ret = send(ETH_Sock2, send_databuf,len, 0);
						if (ret < 0)
						{
								/* 发送失败 关闭连接 */
								closesocket(ETH_Sock2);
								ETH_Sock = 0;
						}
					}
					memset(send_databuf,0,len);
				}
			}
			else
			{
				rt_thread_mdelay(5000);
			}
    }
}

void APP_TCPETHCOMRx_entry(void* parameter)
{
	int ret;
	char recv_data[64];
	uint8_t res=0;
	uint16_t ETH_SerPort;
	uint32_t ETH_SerIP;
	uint8_t state[4];
	uint8_t tag_id[4];
	ETH_SerPort = user_dat[SPort1_Add];
	ETH_SerPort = (ETH_SerPort<<8) + user_dat[SPort2_Add];
	ETH_SerIP = user_dat[SIP4_Add];
	ETH_SerIP = (ETH_SerIP<<8) + user_dat[SIP3_Add];
	ETH_SerIP = (ETH_SerIP<<8) + user_dat[SIP2_Add];
	ETH_SerIP = (ETH_SerIP<<8) + user_dat[SIP1_Add];
	
	while (1)
	{
		if(ETH_Sock != 0)
		{
	
			/*接收*/
			ret = recv(ETH_Sock, recv_data, 63, 0);
			if (ret < 0)
			{
					/* 接收失败，关闭这个连接 */
					closesocket(ETH_Sock);
					ETH_Sock = 0;
					break;
			}
			else if (ret != 0)
			{
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
						send(ETH_Sock, recv_data,ret, 0);
						rt_event_send(&Core_EVENT,EVENT_REBOOT);
					}break;
					case RDCONFIG:
					{
						ret = uc_ReReadStatPackage((uint8_t *)recv_data,ETH_PackNumber++,state);
						send(ETH_Sock, recv_data,ret, 0);
					}break;
					case TAG_WRCONFIG:
					{
						ret = uc_ReSetTagPackage((uint8_t *)recv_data,tag_id,ETH_PackNumber++,state[0]);
						send(ETH_Sock, recv_data,ret, 0);
					}break;
					case TAG_RDCONFIG:
					{
						ret = uc_ReReadTagStatPackage((uint8_t *)recv_data,tag_id,ETH_PackNumber++,state);
						send(ETH_Sock, recv_data,ret, 0);
					}break;
					default:break;
				}
			}
		}
		else
		{
			ETH_Sock = ETHCOM_TCPLink(ETH_SerIP,ETH_SerPort);
			if(ETH_Sock != 0)
				rt_timer_start(timer3);
			else
				rt_thread_mdelay(5000);
		}
	}
}


void APP_TCPETHCOMRx2_entry(void* parameter)
{
	int ret;
	char recv_data[64];
	uint8_t res=0;
	uint16_t ETH_SerPort;
	uint32_t ETH_SerIP;
	uint8_t state[4];
	uint8_t tag_id[4];
	ETH_SerPort = user_dat[SPortx11_Add];
	ETH_SerPort = (ETH_SerPort<<8) + user_dat[SPortx12_Add];
	ETH_SerIP = user_dat[SIPx14_Add];
	ETH_SerIP = (ETH_SerIP<<8) + user_dat[SIPx13_Add];
	ETH_SerIP = (ETH_SerIP<<8) + user_dat[SIPx12_Add];
	ETH_SerIP = (ETH_SerIP<<8) + user_dat[SIPx11_Add];
	
	while (1)
	{
		if(ETH_Sock2 != 0)
		{
			/*接收*/
			ret = recv(ETH_Sock2, recv_data, 63, 0);
			if (ret < 0)
			{
					/* 接收失败，关闭这个连接 */
					closesocket(ETH_Sock2);
					ETH_Sock2 = 0;
					break;
			}
			else if (ret != 0)
			{
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
							send(ETH_Sock2, recv_data,ret, 0);
							rt_event_send(&Core_EVENT,EVENT_REBOOT);
						}break;
						case RDCONFIG:
						{
							ret = uc_ReReadStatPackage((uint8_t *)recv_data,ETH_PackNumber++,state);
							send(ETH_Sock2, recv_data,ret, 0);
						}break;
						case TAG_WRCONFIG:
						{
							ret = uc_ReSetTagPackage((uint8_t *)recv_data,tag_id,ETH_PackNumber++,state[0]);
							send(ETH_Sock2, recv_data,ret, 0);
						}break;
						case TAG_RDCONFIG:
						{
							ret = uc_ReReadTagStatPackage((uint8_t *)recv_data,tag_id,ETH_PackNumber++,state);
							send(ETH_Sock2, recv_data,ret, 0);
						}break;
						default:break;
					}
			}
		}
		else
		{
			ETH_Sock2 = ETHCOM_TCPLink(ETH_SerIP,ETH_SerPort);
			rt_thread_mdelay(5000);
		}
	}
}

void thread_TCP_init(void)
{
	rt_thread_t RT_ETHCOMRx = RT_NULL;
	rt_thread_t RT_ETHCOMTx = RT_NULL;
	rt_thread_t RT_ETHCOMRx2 = RT_NULL;
	/* 创建定时器 3  周期定时器 */
	timer3 = rt_timer_create("timer3", timeout3,RT_NULL, 10000,RT_TIMER_FLAG_PERIODIC);
	/* 初始化一个 mailbox */
	rt_event_init(&ETH_EVENT, "event", RT_IPC_FLAG_FIFO);			
	RT_ETHCOMTx = rt_thread_create("ETHTx",APP_TCPETHCOMTx_entry,RT_NULL,ETHCOM_TXSTACK_SIZE,ETHCOM_TXPRIORITY,ETHCOM_TIMESLICE);	
	RT_ETHCOMRx = rt_thread_create("ETHRx",APP_TCPETHCOMRx_entry,RT_NULL,ETHCOM_RXSTACK_SIZE,ETHCOM_RXPRIORITY,ETHCOM_TIMESLICE);
	//如果有組播地址则再建立一个socket
	if(user_dat[SIPx11_Add] == user_dat[SIP1_Add])
	{
		RT_ETHCOMRx2 = rt_thread_create("ETHRx2",APP_TCPETHCOMRx2_entry,RT_NULL,ETHCOM_RXSTACK_SIZE,ETHCOM_RXPRIORITY,ETHCOM_TIMESLICE);
		if (RT_ETHCOMRx2 != RT_NULL)
		{
			rt_thread_startup(RT_ETHCOMRx2);
		}
	}
	
	if (RT_ETHCOMRx != RT_NULL)
	{
		rt_thread_startup(RT_ETHCOMRx);
		rt_thread_startup(RT_ETHCOMTx);
	}
}


int ETHCOM_TCPLink(uint32_t IP_s, int port_s)
{
		struct sockaddr_in server_addr;
    int sock;
    //建立连接
    /* 创建SOCKET */
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        rt_kprintf("Socket error\n");
        return -1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port_s);
    server_addr.sin_addr.s_addr = IP_s;
    rt_memset(&(server_addr.sin_zero), 0, sizeof(server_addr.sin_zero));

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
		
     /* 连接 */
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1)
    {
        /* 连接失败 */
        rt_kprintf("Connect fail!\n");
        closesocket(sock);
        return 0;
    }
    else
    {
        /* 成功 */
        rt_kprintf("Connect successful\n");
        return sock;
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



