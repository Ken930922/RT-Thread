/*
 * Copyright (c) 2006-2020, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-03-19     CCRFIDZY       the first version
 */
#ifndef APP_APP_UWB_H_
#define APP_APP_UWB_H_

#include <rtthread.h>
#include "drv_spi.h"

#define UWB_STACK_SIZE       512
#define UWB_TXPRIORITY       19
#define UWB_TIMESLICE        5

#define UWB_EXIT_PIN    GET_PIN(A, 8)
#define UWB_REST_PIN    GET_PIN(D, 12)

typedef enum
{
	NET_IF_FT_TD     = 0x01,
	NET_IF_FT_TS     = 0x02,
	NET_IF_FT_TOF    = 0x03,
	NET_IF_FT_TAG    = 0x04,
	NET_IF_FT_WCFG   = 0x05,
	NET_IF_FT_WCFGR  = 0x85, 
	NET_IF_FT_HEART  = 0x88,
}netif_ft_t;


int rt_hw_spi_uwb_init(void);
void APP_UWB_Reset(void);

#endif /* MYAPP_APP_CONFIG_H_ */
