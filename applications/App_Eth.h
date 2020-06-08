/*
 * Copyright (c) 2006-2020, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-03-19     CCRFIDZY       the first version
 */
#ifndef MYAPP_APP_ETH_H_
#define MYAPP_APP_ETH_H_


#include <rtdevice.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include "App_protocol.h"

#define ETHCOM_TXPRIORITY         16
#define ETHCOM_RXPRIORITY         15
#define ETHCOM_TXSTACK_SIZE       1536
#define ETHCOM_RXSTACK_SIZE       1536
#define ETHCOM_TIMESLICE        5

/*设备  控制*/
#define EVENT_HEART             (1)      //  设备 心跳
#define EVENT_UPLOAD		        (1 << 1)   //  设备 上报
#define EVENT_SET_REPLY         (1 << 2)   //  设备 设置应答
#define EVENT_SET_ERROR         (1 << 3)   //  设备 设置错误
#define EVENT_READ_REPLY        (1 << 4)   //  设备 读取应答
#define EVENT_READ_ERROR        (1 << 5)   //  设备 读取错误
#define EVENT_TAG_UPLOAD        (1 << 6)   //  标签 上报
#define EVENT_ALL			          (0xFF)     //  所有事件

extern uint8_t uc_CfgRxbuf[20];

void thread_ETH_init(void);
void thread_TCP_init(void);

#endif /* MYAPP_APP_ETH_H_ */
