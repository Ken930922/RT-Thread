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
#include "App_Core.h"

const char Hello[] __attribute__ ((at(0x08008000))) ={"CCRFID"}; 
extern const unsigned char user_dat[0x4000];
uint32_t NATIVE_ID=0;  //设备ID（同步基站ID）
const unsigned char CRC8Table[] =   //CRC校验
{
  0, 94, 188, 226, 97, 63, 221, 131, 194, 156, 126, 32, 163, 253, 31, 65,
  157, 195, 33, 127, 252, 162, 64, 30, 95, 1, 227, 189, 62, 96, 130, 220,
  35, 125, 159, 193, 66, 28, 254, 160, 225, 191, 93, 3, 128, 222, 60, 98,
  190, 224, 2, 92, 223, 129, 99, 61, 124, 34, 192, 158, 29, 67, 161, 255,
  70, 24, 250, 164, 39, 121, 155, 197, 132, 218, 56, 102, 229, 187, 89, 7,
  219, 133, 103, 57, 186, 228, 6, 88, 25, 71, 165, 251, 120, 38, 196, 154,
  101, 59, 217, 135, 4, 90, 184, 230, 167, 249, 27, 69, 198, 152, 122, 36,
  248, 166, 68, 26, 153, 199, 37, 123, 58, 100, 134, 216, 91, 5, 231, 185,
  140, 210, 48, 110, 237, 179, 81, 15, 78, 16, 242, 172, 47, 113, 147, 205,
  17, 79, 173, 243, 112, 46, 204, 146, 211, 141, 111, 49, 178, 236, 14, 80,
  175, 241, 19, 77, 206, 144, 114, 44, 109, 51, 209, 143, 12, 82, 176, 238,
  50, 108, 142, 208, 83, 13, 239, 177, 240, 174, 76, 18, 145, 207, 45, 115,
  202, 148, 118, 40, 171, 245, 23, 73, 8, 86, 180, 234, 105, 55, 213, 139,
  87, 9, 235, 181, 54, 104, 138, 212, 149, 203, 41, 119, 244, 170, 72, 22,
  233, 183, 85, 11, 136, 214, 52, 106, 43, 117, 151, 201, 74, 20, 246, 168,
  116, 42, 200, 150, 21, 75, 169, 247, 182, 232, 10, 84, 215, 137, 107, 53
};

unsigned char CRC8(uint8_t * pData, uint16_t len)
{
    unsigned char CRC_BUF = 0;
    for( ; len > 0; len--)
    {
        CRC_BUF = CRC8Table[CRC_BUF ^ *pData];
        pData++;
    }
    return(CRC_BUF);
}


/***************************************************** 
描    述：  CRC16 校验子程序 x^16 + x^12 + x^5 + 1 
入口参数：  指向数组指针，校验字节个数 
出口参数：  16 位 CRC 校验码 
******************************************************/ 
const uint16_t crc16_table[256] = {      
     0x0000,0x1021,0x2042,0x3063,0x4084,0x50a5,0x60c6,0x70e7,
     0x8108,0x9129,0xa14a,0xb16b,0xc18c,0xd1ad,0xe1ce,0xf1ef,
     0x1231,0x0210,0x3273,0x2252,0x52b5,0x4294,0x72f7,0x62d6,
     0x9339,0x8318,0xb37b,0xa35a,0xd3bd,0xc39c,0xf3ff,0xe3de,
     0x2462,0x3443,0x0420,0x1401,0x64e6,0x74c7,0x44a4,0x5485,
     0xa56a,0xb54b,0x8528,0x9509,0xe5ee,0xf5cf,0xc5ac,0xd58d,
     0x3653,0x2672,0x1611,0x0630,0x76d7,0x66f6,0x5695,0x46b4,
     0xb75b,0xa77a,0x9719,0x8738,0xf7df,0xe7fe,0xd79d,0xc7bc,
     0x48c4,0x58e5,0x6886,0x78a7,0x0840,0x1861,0x2802,0x3823,
     0xc9cc,0xd9ed,0xe98e,0xf9af,0x8948,0x9969,0xa90a,0xb92b,
     0x5af5,0x4ad4,0x7ab7,0x6a96,0x1a71,0x0a50,0x3a33,0x2a12,
     0xdbfd,0xcbdc,0xfbbf,0xeb9e,0x9b79,0x8b58,0xbb3b,0xab1a,
     0x6ca6,0x7c87,0x4ce4,0x5cc5,0x2c22,0x3c03,0x0c60,0x1c41,
     0xedae,0xfd8f,0xcdec,0xddcd,0xad2a,0xbd0b,0x8d68,0x9d49,
     0x7e97,0x6eb6,0x5ed5,0x4ef4,0x3e13,0x2e32,0x1e51,0x0e70,
     0xff9f,0xefbe,0xdfdd,0xcffc,0xbf1b,0xaf3a,0x9f59,0x8f78,
     0x9188,0x81a9,0xb1ca,0xa1eb,0xd10c,0xc12d,0xf14e,0xe16f,
     0x1080,0x00a1,0x30c2,0x20e3,0x5004,0x4025,0x7046,0x6067,
     0x83b9,0x9398,0xa3fb,0xb3da,0xc33d,0xd31c,0xe37f,0xf35e,
     0x02b1,0x1290,0x22f3,0x32d2,0x4235,0x5214,0x6277,0x7256,
     0xb5ea,0xa5cb,0x95a8,0x8589,0xf56e,0xe54f,0xd52c,0xc50d,
     0x34e2,0x24c3,0x14a0,0x0481,0x7466,0x6447,0x5424,0x4405,
     0xa7db,0xb7fa,0x8799,0x97b8,0xe75f,0xf77e,0xc71d,0xd73c,
     0x26d3,0x36f2,0x0691,0x16b0,0x6657,0x7676,0x4615,0x5634,
     0xd94c,0xc96d,0xf90e,0xe92f,0x99c8,0x89e9,0xb98a,0xa9ab,
     0x5844,0x4865,0x7806,0x6827,0x18c0,0x08e1,0x3882,0x28a3,
     0xcb7d,0xdb5c,0xeb3f,0xfb1e,0x8bf9,0x9bd8,0xabbb,0xbb9a,
     0x4a75,0x5a54,0x6a37,0x7a16,0x0af1,0x1ad0,0x2ab3,0x3a92,
     0xfd2e,0xed0f,0xdd6c,0xcd4d,0xbdaa,0xad8b,0x9de8,0x8dc9,
     0x7c26,0x6c07,0x5c64,0x4c45,0x3ca2,0x2c83,0x1ce0,0x0cc1,
     0xef1f,0xff3e,0xcf5d,0xdf7c,0xaf9b,0xbfba,0x8fd9,0x9ff8,
     0x6e17,0x7e36,0x4e55,0x5e74,0x2e93,0x3eb2,0x0ed1,0x1ef0
}; 

uint16_t CRC16( char * const buf,int count)
{
    unsigned char *ptr = (unsigned char *)buf;
    unsigned short crc_reg = 0xffff;        
    while (count--) 
    {
        crc_reg = (crc_reg << 8) ^ crc16_table[( (crc_reg >> 8) ^ *ptr++ ) & 0xff];
    } 
    return crc_reg;                        
}

	

#define WDT_DEVICE_NAME    "wdt"    /* 看门狗设备名称 */
#define RT_DEVICE_CTRL_WDT_GET_TIMEOUT    (1) /* 获取溢出时间 */
#define RT_DEVICE_CTRL_WDT_SET_TIMEOUT    (2) /* 设置溢出时间 */
#define RT_DEVICE_CTRL_WDT_GET_TIMELEFT   (3) /* 获取剩余时间 */
#define RT_DEVICE_CTRL_WDT_KEEPALIVE      (4) /* 喂狗 */
#define RT_DEVICE_CTRL_WDT_START          (5) /* 启动看门狗 */
#define RT_DEVICE_CTRL_WDT_STOP           (6) /* 停止看门狗 */
rt_device_t wdg_dev; 

static void idle_hook(void)
{
	if(wdg_dev != 0)
    /* 在空闲线程的回调函数里喂狗 */
    rt_device_control(wdg_dev, RT_DEVICE_CTRL_WDT_KEEPALIVE, NULL);
}

int wdt_sample(void)
{
    rt_err_t ret = RT_EOK;
    rt_uint32_t timeout = 32;        /* 溢出时间，单位：秒 */
    char device_name[3]="wdt";

    /* 根据设备名称查找看门狗设备，获取设备句柄 */
    wdg_dev = rt_device_find(device_name);
    if (!wdg_dev)
    {
        rt_kprintf("find %s failed!\n", device_name);
        return RT_ERROR;
    }
    /* 初始化设备 */
    ret = rt_device_init(wdg_dev);
    if (ret != RT_EOK)
    {
        rt_kprintf("initialize %s failed!\n", device_name);
        return RT_ERROR;
    }
    /* 设置看门狗溢出时间 */
    ret = rt_device_control(wdg_dev, RT_DEVICE_CTRL_WDT_SET_TIMEOUT, &timeout);
    if (ret != RT_EOK)
    {
        rt_kprintf("set %s timeout failed!\n", device_name);
        return RT_ERROR;
    }
    /* 启动看门狗 */
    ret = rt_device_control(wdg_dev, RT_DEVICE_CTRL_WDT_START, RT_NULL);
    if (ret != RT_EOK)
    {
        rt_kprintf("start %s failed!\n", device_name);
        return -RT_ERROR;
    }
    /* 设置空闲线程回调函数 */
    rt_thread_idle_sethook(idle_hook);

    return ret;
}


char Core_stack[512];
struct rt_thread APP_Core;
#define THREAD_PRIORITY         25
#define THREAD_TIMESLICE        5
struct rt_event Core_EVENT;

#define LED_R GET_PIN(E, 4)
#define LED_G GET_PIN(E, 6)

/* 线程 1 的入口函数 */
static void APP_Core_entry(void *parameter)
{
	rt_pin_mode(LED_R, PIN_MODE_OUTPUT);
	rt_pin_mode(LED_G, PIN_MODE_OUTPUT);
	uint32_t CMD;
	while (1)
	{
		if (rt_event_recv(&Core_EVENT, EVENT_ALL,
													RT_EVENT_FLAG_OR | RT_EVENT_FLAG_CLEAR,
													RT_WAITING_FOREVER, &CMD) == RT_EOK)
			{
				switch(CMD)
				{
					case EVENT_REBOOT:
					{
						HAL_NVIC_SystemReset();
					}break;
					case EVENT_REDLED:
					{
						rt_pin_write(LED_R, PIN_LOW);
						rt_thread_mdelay(10);

					}break;
					case EVENT_GREENLED:
					{
						rt_pin_write(LED_G, PIN_LOW);
						rt_thread_mdelay(10);
					}break;
				}
				rt_pin_write(LED_R, PIN_HIGH);
				rt_pin_write(LED_G, PIN_HIGH);
			}
	}
}


void APP_Core_Init(void)
{
	NATIVE_ID = user_dat[50];
  NATIVE_ID = (NATIVE_ID<<8)+user_dat[51];
  NATIVE_ID = (NATIVE_ID<<8)+user_dat[52];
  NATIVE_ID = (NATIVE_ID<<8)+user_dat[53];
	
	/* 初始化一个 mailbox */
	rt_event_init(&Core_EVENT, "event", RT_IPC_FLAG_FIFO);			

	/* 初始化线程 2，名称是 thread2，入口是 thread2_entry */
	rt_thread_init(&APP_Core,"APP_Core",APP_Core_entry,RT_NULL,&Core_stack[0],sizeof(Core_stack),THREAD_PRIORITY - 1, THREAD_TIMESLICE);
	rt_thread_startup(&APP_Core);

}

