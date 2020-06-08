/*
 * Copyright (c) 2006-2020, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-04-22     CCRFIDZY       the first version
 */
#include <rtthread.h>
#include "App_Config.h"
#include "APP_Core.h"
#include "drv_flash.h"
#include "netdev.h"

int CONFIG_Sock;  //sock句柄    
extern struct sockaddr_in Eth_client[5];
extern uint16_t ETH_LocalPort;
extern uint16_t ETH_SerPort;
extern const unsigned char user_dat[0x4000];


void APP_CONFIGCOMTx_entry(void* parameter)
{
	int bytes_read;
	ConfigData *recv_data;
	socklen_t addr_len;
	struct sockaddr_in Configclient_addr;

	if (CONFIG_Sock != NULL)
	{
		/* 分配用于存放接收数据的缓冲 */
		recv_data = rt_malloc(sizeof(ConfigData));
		if (recv_data == RT_NULL)
		{
				rt_kprintf("No memory\n");
		}
		addr_len = sizeof(struct sockaddr);
	}
	while (1)
	{
		bytes_read = recvfrom(CONFIG_Sock, recv_data, 170, 0,(struct sockaddr *)&Configclient_addr, &addr_len);
		if(recv_data->head == 0x5A)
		{
			switch (recv_data->cmd)
			{
				case 0:
				{
					//解析
					recv_data->cmd=1;
					recv_data->loc_IP[0] = user_dat[CIP1_Add];
					recv_data->loc_IP[1] = user_dat[CIP2_Add];
					recv_data->loc_IP[2] = user_dat[CIP3_Add];
					recv_data->loc_IP[3] = user_dat[CIP4_Add];
					recv_data->loc_mask[0] = user_dat[Mask1_Add];
					recv_data->loc_mask[1] = user_dat[Mask2_Add];
					recv_data->loc_mask[2] = user_dat[Mask3_Add];
					recv_data->loc_mask[3] = user_dat[Mask4_Add];
					recv_data->loc_gw[0] = user_dat[Gateway1_Add];
					recv_data->loc_gw[1] = user_dat[Gateway2_Add];
					recv_data->loc_gw[2] = user_dat[Gateway3_Add];
					recv_data->loc_gw[3] = user_dat[Gateway4_Add];
					recv_data->ser_IP[0] = user_dat[SIP1_Add];
					recv_data->ser_IP[1] = user_dat[SIP2_Add];
					recv_data->ser_IP[2] = user_dat[SIP3_Add];
					recv_data->ser_IP[3] = user_dat[SIP4_Add];
					recv_data->loc_Port[0] = user_dat[CPort1_Add];
					recv_data->loc_Port[1] = user_dat[CPort2_Add];
					recv_data->ser_Port[0] = user_dat[SPort1_Add];
					recv_data->ser_Port[1] = user_dat[SPort2_Add];
					recv_data->model = user_dat[WorkMode_Add];; 	//UDP
					recv_data->ID[0] = (uint8_t)(NATIVE_ID>>24);
					recv_data->ID[1] = (uint8_t)(NATIVE_ID>>16);
					recv_data->ID[2] = (uint8_t)(NATIVE_ID>>8);
					recv_data->ID[3] = (uint8_t)NATIVE_ID;
					recv_data->Name[0] = 'C';
					recv_data->Name[1] = 'C';
					recv_data->Name[2] = 'R';
					recv_data->Name[3] = 'F';
					recv_data->Name[4] = 'I';
					recv_data->Name[5] = 'D';
					recv_data->Name[6] = '0';
					recv_data->Name[7] = (uint8_t)NATIVE_ID+0x30;
					recv_data->page[0] = 3;
					recv_data->page[1] = 1;
					recv_data->page[2] = 1;
					recv_data->DHCP = user_dat[DHCP_Add];
					recv_data->ser_IP2[0] = user_dat[SIPx11_Add];
					recv_data->ser_IP2[1] = user_dat[SIPx12_Add];
					recv_data->ser_IP2[2] = user_dat[SIPx13_Add];
					recv_data->ser_IP2[3] = user_dat[SIPx14_Add];
					recv_data->ser_Port2[0] = user_dat[SIPx11_Add];
					recv_data->ser_Port2[1] = user_dat[SIPx12_Add];
					memset(recv_data->other3,0,63);
					recv_data->other3[58] = 1;
					recv_data->other3[59] = 1;
//					Configclient_addr.sin_addr.s_addr = IPADDR_BROADCAST;
					sendto(CONFIG_Sock, recv_data, bytes_read, 0,(struct sockaddr *)&Configclient_addr, sizeof(struct sockaddr));
				}break;
				case 2:
				{
					uint8_t flash_buff[100];
					recv_data->cmd=1;
					stm32_flash_read(Memery_Start_Address,flash_buff,100);
					flash_buff[DHCP_Add] = recv_data->DHCP;
					flash_buff[CIP1_Add] = recv_data->loc_IP[0];
					flash_buff[CIP2_Add] = recv_data->loc_IP[1];
					flash_buff[CIP3_Add] = recv_data->loc_IP[2];
					flash_buff[CIP4_Add] = recv_data->loc_IP[3];
					flash_buff[CPort1_Add] = recv_data->loc_Port[0];
					flash_buff[CPort2_Add] = recv_data->loc_Port[1];
					flash_buff[WorkMode_Add] = recv_data->model;
					flash_buff[Mask1_Add] = recv_data->loc_mask[0];
					flash_buff[Mask2_Add] = recv_data->loc_mask[1];
					flash_buff[Mask3_Add] = recv_data->loc_mask[2];
					flash_buff[Mask4_Add] = recv_data->loc_mask[3];
					flash_buff[Gateway1_Add] = recv_data->loc_gw[0];
					flash_buff[Gateway2_Add] = recv_data->loc_gw[1];
					flash_buff[Gateway3_Add] = recv_data->loc_gw[2];
					flash_buff[Gateway4_Add] = recv_data->loc_gw[3];
					flash_buff[SIP1_Add] = recv_data->ser_IP[0];
					flash_buff[SIP2_Add] = recv_data->ser_IP[1];
					flash_buff[SIP3_Add] = recv_data->ser_IP[2];
					flash_buff[SIP4_Add] = recv_data->ser_IP[3];
					flash_buff[SPort1_Add] = recv_data->ser_Port[0];
					flash_buff[SPort2_Add] = recv_data->ser_Port[1];
					flash_buff[SIPx11_Add] = recv_data->ser_IP2[0];
					flash_buff[SIPx12_Add] = recv_data->ser_IP2[1];
					flash_buff[SIPx13_Add] = recv_data->ser_IP2[2];
					flash_buff[SIPx14_Add] = recv_data->ser_IP2[3];
					flash_buff[SPortx11_Add] = recv_data->ser_Port2[0];
					flash_buff[SPortx12_Add] = recv_data->ser_Port2[1];
					stm32_flash_erase(Memery_Start_Address,1);
					stm32_flash_write(Memery_Start_Address,flash_buff,100);
//					sendto(CONFIG_Sock, recv_data, bytes_read, 0,(struct sockaddr *)&Configclient_addr, sizeof(struct sockaddr));
//					memset(recv_data,0,bytes_read);
					HAL_NVIC_SystemReset();
				}break;
			}
		}
	}
}

void thread_CONFIGCOM_init(void)
{
	rt_thread_t RT_CONFIGCOM = RT_NULL;
	
	RT_CONFIGCOM = rt_thread_create("ConfigCom",APP_CONFIGCOMTx_entry,RT_NULL,CONFIGCOM_STACK_SIZE,CONFIGCOM_PRIORITY,CONFIGCOM_TIMESLICE);		
	if ((CONFIG_Sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
	{
			rt_kprintf("Socket error\n"); 
	}
	else
	{
		struct sockaddr_in server_addr;			
		/* 初始化预连接的服务端地址 */
		server_addr.sin_family = AF_INET;
		server_addr.sin_port = htons(1092);
		server_addr.sin_addr.s_addr = IPADDR_ANY;
		rt_memset(&(server_addr.sin_zero), 0, sizeof(server_addr.sin_zero));
		/* 绑定socket到服务端地址 */
		if (bind(CONFIG_Sock, (struct sockaddr *)&server_addr,sizeof(struct sockaddr)) == -1)
		{
				/* 绑定地址失败 */
				closesocket(CONFIG_Sock);
				rt_kprintf("Bind error\n");
		}
		else
		{
			int flag=1;
			setsockopt(CONFIG_Sock, SOL_SOCKET, SO_BROADCAST, &flag, sizeof(flag));
		}
	 }
	if (RT_CONFIGCOM != RT_NULL)
	{
		 rt_thread_startup(RT_CONFIGCOM);
	}
}



