/* Flash OS Routines (Automagically Generated)
 * Copyright (c) 2009-2015 ARM Limited
 */
#include "swd.h"

static const uint32_t nRF52832AA_FLM[] = {
    0xE00ABE00, 0x062D780D, 0x24084068, 0xD3000040, 0x1E644058, 0x1C49D1FA, 0x2A001E52, 0x4770D1F2,
    0x47702000, 0x47702000, 0x4c2bb570, 0x60202002, 0x20014929, 0x60083108, 0x68284d28, 0xd00207c0, 
    0x60202000, 0xf000bd70, 0xe7f6f833, 0x4c22b570, 0x60212102, 0x2f10f1b0, 0x491fd303, 0x31102001, 
    0x491de001, 0x60081d09, 0xf0004d1c, 0x6828f821, 0xd0fa07c0, 0x60202000, 0xe92dbd70, 0xf8df41f0, 
    0x088e8058, 0x46142101, 0xf8c84605, 0x4f131000, 0xc501cc01, 0x07c06838, 0x1e76d007, 0x2100d1f8, 
    0x1000f8c8, 0xe8bd4608, 0xf00081f0, 0xe7f1f801, 0x6800480b, 0x00fff010, 0x490ad00c, 0x29006809, 
    0x4908d008, 0x31fc4a08, 0xd00007c3, 0x1d09600a, 0xd1f90840, 0x00004770, 0x4001e504, 0x4001e400, 
    0x40010404, 0x40010504, 0x6e524635, 0x00000000, 
};

const program_target_t flash_nrf52 = {
    .init = 0x20000021,
    .uninit = 0x20000025,
    .erase_chip = 0x20000029,
    .erase_sector = 0x2000004D,
    .program_page = 0x2000007B,
    {
        .breakpoint = 0x20000001,
        .static_base = 0x20000020 + 0x00000150,
        .stack_pointer = 0x20001000
    },
    .program_buffer = 0x20000400,
    .algo_start = 0x20000000,
    .algo_size = 0,
    .algo_blob = nRF52832AA_FLM,
    .program_buffer_size = 0x00000400 // should be USBD_MSC_BlockSize
};


static const uint32_t STM32F4_FLM[] = {
				0xE00ABE00, 0x062D780D, 0x24084068, 0xD3000040, 0x1E644058, 0x1C49D1FA, 0x2A001E52, 0x4770D1F2,
        0x03004601, 0x28200E00, 0x0940D302, 0xE0051D00, 0xD3022810, 0x1CC00900, 0x0880E000, 0xD50102C9,
        0x43082110, 0x48424770, 0x60414940, 0x60414941, 0x60012100, 0x22F068C1, 0x60C14311, 0x06806940,
        0x483ED406, 0x6001493C, 0x60412106, 0x6081493C, 0x47702000, 0x69014836, 0x43110542, 0x20006101,
        0xB5104770, 0x69014832, 0x43212404, 0x69016101, 0x431103A2, 0x49336101, 0xE0004A30, 0x68C36011,
        0xD4FB03DB, 0x43A16901, 0x20006101, 0xB530BD10, 0xFFB6F7FF, 0x68CA4926, 0x431A23F0, 0x240260CA,
        0x690A610C, 0x0E0006C0, 0x610A4302, 0x03E26908, 0x61084310, 0x4A214823, 0x6010E000, 0x03ED68CD,
        0x6908D4FB, 0x610843A0, 0x060068C8, 0xD0030F00, 0x431868C8, 0x200160C8, 0xB570BD30, 0x1CC94D14,
        0x68EB0889, 0x26F00089, 0x60EB4333, 0x612B2300, 0xE0174B15, 0x431C692C, 0x6814612C, 0x68EC6004,
        0xD4FC03E4, 0x0864692C, 0x612C0064, 0x062468EC, 0xD0040F24, 0x433068E8, 0x200160E8, 0x1D00BD70,
        0x1F091D12, 0xD1E52900, 0xBD702000, 0x45670123, 0x40023C00, 0xCDEF89AB, 0x00005555, 0x40003000,
        0x00000FFF, 0x0000AAAA, 0x00000201, 0x00000000
};

const program_target_t flash_stm32f4 = {
    .init = 0x20000047,
    .uninit = 0x20000075,
    .erase_chip = 0x20000083,
    .erase_sector = 0x200000AF,
    .program_page = 0x200000FB,
    {
        .breakpoint = 0x20000001,
        .static_base = 0x20000400,
        .stack_pointer = 0x20001000
    },
    .program_buffer = 0x20000400,
    .algo_start = 0x20000000,
    sizeof(STM32F4_FLM),  // prog_blob size
    STM32F4_FLM,  // address of prog_blob
    .program_buffer_size = 0x00000400 // should be USBD_MSC_BlockSize
};
