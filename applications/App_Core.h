/*
 * Copyright (c) 2006-2020, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-03-17     CCRFIDZY       the first version
 */
#ifndef MYAPP_APP_CORE_H_
#define MYAPP_APP_CORE_H_

#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include "stm32f40x_swd.h"
#include "swd_pin.h"
#include "swd_flash.h"
#include "App_Config.h"
#include "App_UWB.h"
#include "queue.h"
#include "fal.h"

#define Memery_Start_Address     ((uint32_t)0x0800C000)
#define SoftVersionADD    0
#define BLE_SIZE    SoftVersionADD+1
#define BLE_DES   	BLE_SIZE+4   //没用到

#define BOOT_FLAG    		 BLE_DES+2   //5
#define BOOT_SIZE    		 BOOT_FLAG+1 //7
#define UWB_SIZE     		 BOOT_SIZE+4 //8
#define DHCP_Add         UWB_SIZE+4  //16
#define CIP1_Add    		 DHCP_Add+1
#define CIP2_Add			   CIP1_Add+1   
#define CIP3_Add			   CIP2_Add+1   
#define CIP4_Add			   CIP3_Add+1  
#define CPort1_Add			 CIP4_Add+1   
#define CPort2_Add			 CPort1_Add+1   
#define WorkMode_Add		 CPort2_Add+1   
#define Mask1_Add			   WorkMode_Add+1   
#define Mask2_Add			   Mask1_Add+1   
#define Mask3_Add			   Mask2_Add+1   
#define Mask4_Add			   Mask3_Add+1  
#define Gateway1_Add     Mask4_Add+1   
#define Gateway2_Add	   Gateway1_Add+1   
#define Gateway3_Add	   Gateway2_Add+1   
#define Gateway4_Add	   Gateway3_Add+1  
#define SIP1_Add    		 Gateway4_Add+1
#define SIP2_Add			   SIP1_Add+1   
#define SIP3_Add			   SIP2_Add+1   
#define SIP4_Add			   SIP3_Add+1 
#define SPort1_Add			 SIP4_Add+1   
#define SPort2_Add			 SPort1_Add+1
#define SIPx11_Add    	 SPort2_Add+1
#define SIPx12_Add			 SIPx11_Add+1   
#define SIPx13_Add			 SIPx12_Add+1   
#define SIPx14_Add			 SIPx13_Add+1 
#define SPortx11_Add	   SIPx14_Add+1   
#define SPortx12_Add		 SPortx11_Add+1
#define UWBID1_Add	   	 SPortx12_Add+1   
#define UWBID2_Add		   UWBID1_Add+1
#define UWBID3_Add	     UWBID2_Add+1   
#define UWBID4_Add		   UWBID3_Add+1
#define UWBHIGHT1_Add	   UWBID4_Add+1   
#define UWBHIGHT2_Add		 UWBHIGHT1_Add+1
#define ConfigTable_Add  UWBHIGHT2_Add+1


#define SoftVersion 0x02

//app 使能配置表
#define APP_USING_WEBNET
#define APP_USING_ETH
#define APP_USING_BLE
#define APP_USING_UWB

/*设备  控制*/
#define EVENT_REBOOT            (1)      	 //  重启
#define EVENT_REDLED            (1<<1)     //  红灯
#define EVENT_GREENLED          (1<<2)     //  绿灯
#define EVENT_ALL			          (0xFF)     //  所有事件

extern struct rt_event Core_EVENT;
extern uint32_t NATIVE_ID;

unsigned char CRC8(uint8_t * pData, uint16_t len);
uint16_t CRC16( char * const buf,int count);
int wdt_sample(void);
void APP_Core_Init(void);

#endif /* MYAPP_APP_CORE_H_ */
