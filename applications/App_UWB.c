/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-11-27     SummerGift   add spi flash port file
 */

#include <rtthread.h>
#include "drv_spi.h"
#include "App_UWB.h"
#include "APP_Core.h"
#include "drv_flash.h"

rt_uint8_t RXBUF[128];
rt_uint8_t TXBUF[128];

static uint8_t Config_Table[Config_Table_Len];


struct rt_spi_device *UWB_spidev;
static rt_sem_t UWB_sem = RT_NULL;
extern struct rt_event ETH_EVENT;
void APP_UWB_read_entry(void* parameter);

void App_UWB_Callback(void *args)
{
	if(rt_pin_read(UWB_EXIT_PIN) == PIN_HIGH)
	{
		rt_sem_release(UWB_sem);
	}
}

void UWB_CONFIG_Init(void)
{
	Config_Table[0] = (uint8_t)(NATIVEDEF_ID>>24);
	Config_Table[1] = (uint8_t)(NATIVEDEF_ID>>16);
	Config_Table[2] = (uint8_t)(NATIVEDEF_ID>>8);
	Config_Table[3] = (uint8_t)NATIVEDEF_ID;
	Config_Table[4] = UWBMODE_DEF;
	Config_Table[5] = UWBHEIGH_DEF;	
	APP_flash_upload(Config_Table,ConfigTable_Add,Config_Table_Len);
}

int rt_hw_spi_uwb_init(void)
{
	uint32_t ID=0;
	rt_thread_t RT_UWB = RT_NULL;
	struct rt_spi_configuration cfg;
	rt_pin_mode(UWB_EXIT_PIN, PIN_MODE_INPUT);
	rt_pin_mode(UWB_REST_PIN, PIN_MODE_OUTPUT);
	
	stm32_flash_read(Memery_Start_Address+ConfigTable_Add,Config_Table,Config_Table_Len);
	ID = Config_Table[0];
	ID = Config_Table[1] + (ID<<8);
	ID = Config_Table[2] + (ID<<8);
	ID = Config_Table[3] + (ID<<8);
	if((ID == 0)||(ID == 0xFFFFFFFF))
	{
		UWB_CONFIG_Init();
	}
	
	
	rt_pin_attach_irq(UWB_EXIT_PIN, PIN_IRQ_MODE_RISING, App_UWB_Callback, RT_NULL);
	
	rt_hw_spi_device_attach("spi2", "spi20", GPIOD, GPIO_PIN_0);

	UWB_spidev = (struct rt_spi_device *)rt_device_find("spi20");
	
	cfg.data_width = 8;
	cfg.mode = RT_SPI_MASTER | RT_SPI_MODE_0 | RT_SPI_MSB;
	cfg.max_hz = 20 * 1000 *1000;                           /* 20M */

	if (UWB_spidev)
	{
		rt_spi_configure(UWB_spidev, &cfg);
		UWB_sem = rt_sem_create("UWBsem", 0, RT_IPC_FLAG_FIFO);
		RT_UWB = rt_thread_create("UWB_readdata",APP_UWB_read_entry,RT_NULL,UWB_STACK_SIZE,UWB_TXPRIORITY,UWB_TIMESLICE);					
		if(RT_UWB != RT_NULL)
    {
			rt_thread_startup(RT_UWB);
			APP_UWB_Reset();
    }
	}
	return RT_EOK;
}

void APP_UWB_Reset(void)
{
	rt_pin_write(UWB_REST_PIN, PIN_LOW);
	rt_thread_mdelay(10);
	rt_pin_write(UWB_REST_PIN, PIN_HIGH);
	rt_thread_mdelay(10);
}

void APP_UWB_read_entry(void* parameter)
{
	rt_err_t result;
	rt_uint16_t len=0;
	rt_uint8_t count=0;
	
	if(UWB_spidev != RT_NULL)
	{
		rt_pin_irq_enable(UWB_EXIT_PIN, PIN_IRQ_ENABLE);
	}
	while(1)
	{
		result = rt_sem_take(UWB_sem, 1000);
		if (result == RT_EOK)
		{
			count=0;
			rt_spi_transfer(UWB_spidev,&TXBUF,RXBUF, 128);
			len = RXBUF[1]+2;
//			for(int i=0;i<=len;i++)
//				rt_kprintf("%x ",RXBUF[i]);
//			rt_kprintf("\r\n"); 
			memset(TXBUF,0xFF,40);
			if(RXBUF[2] == 0x11)
			{
				//当内容为 启动帧时 配置UWB,否则压入队列中
				if(RXBUF[3] == 0x07)
				{
					int i =0;
					TXBUF[0] = 0x00;
					TXBUF[1] = 0x26;
					TXBUF[2] = 0x11; //
					TXBUF[3] = 0x05; //帧类型
					for(i = 0;i<Config_Table_Len;i++)
						TXBUF[4+i] = Config_Table[i];
					for(i = 10;i<=35;i++)
						TXBUF[i] = 0x00;
					TXBUF[36] = 0x12;
					TXBUF[37] = 0x34;
					TXBUF[38] = 0x56;
					TXBUF[39] = 0x78;
					rt_sem_release(UWB_sem);
					memset(RXBUF,0,len);
				}
				else 
				{
					if(Fulqueue(ETH_SEND_QUEUE) == 1)
					{
						// 缓存满，需要腾出一个缓存空间。
						Dequeue(ETH_SEND_QUEUE,  RXBUF);
						EnQueue(ETH_SEND_QUEUE,  RXBUF);
					}
					else
					{
						EnQueue(ETH_SEND_QUEUE , RXBUF);
					}	
					memset(RXBUF,0,len);
					rt_event_send(&ETH_EVENT,EVENT_TAG_UPLOAD);
					rt_event_send(&Core_EVENT,EVENT_REDLED);
				}
			}
		}
		else
		{
			if(rt_pin_read(UWB_EXIT_PIN) == PIN_HIGH)
			{
				rt_sem_release(UWB_sem);
			}
			else
			{
				count++;
				if(count > 10)
				{
					count = 0;
					APP_UWB_Reset();
				}
			}
		}
	}
}
