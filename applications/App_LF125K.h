/*
 * Copyright (c) 2006-2020, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-03-19     CCRFIDZY       the first version
 */
#ifndef MYAPP_APP_LF125K_H_
#define MYAPP_APP_LF125K_H_

extern uint8_t TagSet_TimerOutFlog;         //LF ≥¨ ±±Í÷æ
extern uint8_t LfConfigBuf[20];

void LF125K_Init(void);
uint8_t LfSendMsg(void);
uint8_t BSP_LfConfigTagMsg(uint8_t* pbuf);
void Change_Resistor(uint8_t Data);

#endif /* MYAPP_APP_LF125K_H_ */
