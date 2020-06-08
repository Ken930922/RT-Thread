/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-11-06     SummerGift   first version
 * 2018-11-19     flybreak     add stm32f407-atk-explorer bsp
 */

#include "drv_flash.h"
#include "App_WebNet.h"
#include "App_Core.h"
#include "App_Eth.h"
#include "App_LF125K.h"
#include "SWD_flash.h"
#include "stm32f40x_swd.h"
#include "drv_flash.h"
#include "netdev.h"
#include "stdio.h"

#define UWBEN1_PIN    GET_PIN(D, 14)
#define UWBEN2_PIN    GET_PIN(D, 15)
#define UWB_REST_PIN  GET_PIN(D, 12)




const unsigned char user_dat[0x4000] __attribute__((at(0x0800C000)));
//放到第三扇区 16k大小
void Flash_DeInit(uint8_t *databuf)
{
	memset(databuf,0,50);
	databuf[SoftVersionADD] = SoftVersion;
	databuf[DHCP_Add] = 1;
	databuf[CIP1_Add] = 192;
	databuf[CIP2_Add] = 168;
	databuf[CIP3_Add] = 123;
	databuf[CIP4_Add] = 30;
	databuf[CPort1_Add] = 0x0f;
	databuf[CPort2_Add] = 0xA1;
	databuf[WorkMode_Add] = 2;
	databuf[Mask1_Add] = 0;
	databuf[Mask2_Add] = 0;
	databuf[Mask3_Add] = 0;
	databuf[Mask4_Add] = 0;
	databuf[Gateway1_Add] = 192;
	databuf[Gateway2_Add] = 168;
	databuf[Gateway3_Add] = 123;
	databuf[Gateway4_Add] = 1;
	databuf[SIP1_Add] = 255;
	databuf[SIP2_Add] = 255;
	databuf[SIP3_Add] = 255;
	databuf[SIP4_Add] = 255;
	databuf[SPort1_Add] = 0x17;
	databuf[SPort2_Add] = 0x70;
}


int main(void)
{
		uint8_t FLASH_BUFF[100];
    /*---版本校验----*/
		stm32_flash_read(Memery_Start_Address,FLASH_BUFF,100);
    if(FLASH_BUFF[SoftVersionADD] != SoftVersion)
    {
			Flash_DeInit(FLASH_BUFF);
			stm32_flash_erase(Memery_Start_Address,100);
			stm32_flash_write(Memery_Start_Address,FLASH_BUFF,100);
    }
    rt_kprintf("This Soft Version is V%d .Today is 5.18\r\n", FLASH_BUFF[SoftVersionADD]);

//		wdt_sample();
		
		thread_CONFIGCOM_init();
		
		#if defined(APP_USING_WEBNET)
		WebNet_Start();
		#endif

		#if defined(APP_USING_ETH)
		if(FLASH_BUFF[WorkMode_Add] == 2)//UDP
			thread_ETH_init();
		else
			thread_TCP_init();
		#endif
		
		#if defined(APP_USING_UWB)
		rt_pin_mode(UWBEN1_PIN, PIN_MODE_OUTPUT);
		rt_pin_mode(UWBEN2_PIN, PIN_MODE_OUTPUT);
		rt_pin_write(UWBEN1_PIN, PIN_HIGH);
		rt_pin_write(UWBEN2_PIN, PIN_HIGH);
		SWD_DeInit(SWD1);
		SWD_DeInit(SWD2);
		#endif
			
		rt_hw_spi_uwb_init();
		
		APP_Core_Init();

    while (1)
    {
			rt_event_send(&Core_EVENT,EVENT_GREENLED);
			rt_thread_mdelay(500);
    }
}


#define RT_APP_PART_ADDR 0x08004000
/**
 * Function    ota_app_vtor_reconfig
 * Description Set Vector Table base location to the start addr of app(RT_APP_PART_ADDR).
*/
static int ota_app_vtor_reconfig(void)
{
    #define NVIC_VTOR_MASK   0x3FFFFF80
    /* Set the Vector Table base location by user application firmware definition */
    SCB->VTOR = RT_APP_PART_ADDR & NVIC_VTOR_MASK;

    return 0;
}
INIT_BOARD_EXPORT(ota_app_vtor_reconfig);

