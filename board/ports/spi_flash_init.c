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
#include "spi_flash.h"
#include "spi_flash_sfud.h"
#include "drv_spi.h"
#include "fal.h"
#include "dfs_fs.h"

#define FS_PARTITION_NAME  "system"

#if defined(BSP_USING_SPI_FLASH)
static int rt_hw_spi_flash_init(void)
{
	struct rt_spi_device *spi_dev;
	rt_uint8_t TXBUF[7];
  rt_uint8_t RXBUF[6];
    __HAL_RCC_GPIOA_CLK_ENABLE();

    rt_hw_spi_device_attach("spi3", "spi30", GPIOD, GPIO_PIN_6);

    if (RT_NULL == rt_sfud_flash_probe("SST26VF016B", "spi30"))
    {
       return -RT_ERROR;
    };
		
		spi_dev = (struct rt_spi_device *)rt_device_find("spi30");
    if (spi_dev)
    {
        /* ��ʽ1��ʹ�� rt_spi_send_then_recv()���������ȡID */
				TXBUF[0]= 0x06;
				rt_spi_send_then_recv(spi_dev, TXBUF, 1, RXBUF, 1);
				TXBUF[0]= 0x42;TXBUF[1]= 0x0;TXBUF[2]= 0x0;TXBUF[3]= 0x0;TXBUF[4]= 0x0;TXBUF[5]= 0x0;TXBUF[6]= 0x0;
				rt_spi_send_then_recv(spi_dev, TXBUF, 7, RXBUF, 1);
    }
		
		

    fal_init();
    /* �� spi flash ����Ϊ "filesystem" �ķ����ϴ���һ�����豸 */
    struct rt_device *flash_dev = fal_blk_device_create(FS_PARTITION_NAME);
    if (flash_dev == NULL)
    {
        rt_kprintf("Can't create a block device on '%s' partition.\r\n", FS_PARTITION_NAME);
    }

    /* ���� spi flash ����Ϊ "filesystem" �ķ����ϵ��ļ�ϵͳ */
    if (dfs_mount(flash_dev->parent.name, "/", "elm", 0, 0) != 0)
    {
        rt_kprintf("Failed to initialize filesystem!\r\n");
        rt_kprintf("You should create a filesystem on the block device first!\r\n");
    }

    return RT_EOK;
}
INIT_COMPONENT_EXPORT(rt_hw_spi_flash_init);
#endif

