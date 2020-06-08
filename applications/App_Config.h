/*
 * Copyright (c) 2006-2020, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-03-19     CCRFIDZY       the first version
 */
#ifndef MYAPP_APP_CONFIG_H_
#define MYAPP_APP_CONFIG_H_


#include <rtdevice.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include "App_protocol.h"

#define CONFIGCOM_PRIORITY         13
#define CONFIGCOM_STACK_SIZE       2048
#define CONFIGCOM_TIMESLICE        5

typedef struct ConfigData_Struct
{
	uint8_t head;        //帧头 5A
	uint8_t len;				 //长度
	uint8_t cmd;				 //CMD
	uint8_t loc_IP[4];	 //本地IP
	uint8_t loc_mask[4]; //本地掩码
	uint8_t loc_gw[4];	 //本地网关
	uint8_t ser_IP[4];   //服务器IP
	uint8_t loc_Port[2]; //本地端口
	uint8_t ser_Port[2]; //服务器端口
	uint8_t model;
	uint8_t unknow[12];
	uint8_t ID[4];
	uint8_t Baud;
	uint8_t Name[9];
	uint8_t other1[2];
	uint8_t page[3];
	uint8_t other2[4];
	uint8_t DHCP;
	uint8_t other3[63];
	uint8_t ser_IP2[4];
	uint8_t ser_Port2[2];
	uint8_t ser_IP3[4];
	uint8_t ser_Port3[2];
	uint8_t ser_IP4[4];
	uint8_t ser_Port4[2];
	uint8_t ser_IP5[4];
	uint8_t ser_Port5[2];
	uint8_t ser_IP6[4];
	uint8_t ser_Port6[2];
	uint8_t ser_IP7[4];
	uint8_t ser_Port7[2];
  uint8_t ser_IP8[4];
	uint8_t ser_Port8[2];
	uint8_t other4[5];
}ConfigData;



void thread_CONFIGCOM_init(void);


#endif /* MYAPP_APP_CONFIG_H_ */
