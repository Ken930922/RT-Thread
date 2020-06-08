#ifndef __SWD_FLASH_H__
#define __SWD_FLASH_H__

#include <stdint.h>

#include "error.h"

#define UWB_SWD  1
#define BLE_SWD  2

error_t target_flash_init(uint8_t ch,uint32_t flash_start);
error_t target_flash_uninit(uint8_t ch);
error_t target_flash_program_page(uint8_t ch,uint32_t addr, const uint8_t *buf, uint32_t size);
error_t target_flash_erase_sector(uint8_t ch,uint32_t addr);
error_t target_flash_erase_chip(uint8_t ch);


#endif // __SWD_FLASH_H__
