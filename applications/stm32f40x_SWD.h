/**
  ******************************************************************************
  * @file  stm32f40x_swd.h
  * @author  Ken
  * @version  V1.0.0
  * @date  2020-4-28
  * @brief  This file contains all the functions prototypes for the SWD 
  *         firmware library.
  ******************************************************************************
  **/ 

/* Define to prevent recursive inclusion -------------------------------------*/

#ifndef __STM32F40X_SWD_H
#define __STM32F40X_SWD_H

#include "stm32f4xx_hal.h"

#define UWB_SWD  1
#define BLE_SWD  2
// DAP Transfer Response
#define DAP_TRANSFER_OK                 (1<<0)
#define DAP_TRANSFER_WAIT               (1<<1)
#define DAP_TRANSFER_FAULT              (1<<2)
#define DAP_TRANSFER_ERROR              (1<<3)
#define DAP_TRANSFER_MISMATCH           (1<<4)
#define DAP_TRANSFER_TIMEOUT            (1<<5)
// DAP Transfer Request
#define DAP_TRANSFER_APnDP              (1<<0)
#define DAP_TRANSFER_RnW                (1<<1)
#define DAP_TRANSFER_A2                 (1<<2)
#define DAP_TRANSFER_A3                 (1<<3)
#define DAP_TRANSFER_MATCH_VALUE        (1<<4)
#define DAP_TRANSFER_MATCH_MASK         (1<<5)
// SWD register access
#define SWD_REG_AP        (1)
#define SWD_REG_DP        (0)
#define SWD_REG_R        (1<<1)
#define SWD_REG_W        (0<<1)
#define SWD_REG_ADR(a)    (a & 0x0c)

/** 
  * @brief  SWD Hardware structure definition 
  */

typedef struct __SWDHardware_TypeDef_Struct
{
		GPIO_TypeDef      *CLK_GPIOx;
		uint16_t          CLK_GPIO_Pin;
		GPIO_TypeDef  		*DIO_GPIOx;
		uint16_t          DIO_GPIO_Pin;
		GPIO_TypeDef  		*RST_GPIOx;
		uint16_t          RST_GPIO_Pin;
}SWDHardware_TypeDef_Struct;

/** 
  * @brief  SWD Memory structure definition  
  */
typedef struct
{
    uint32_t breakpoint;
    uint32_t static_base;
    uint32_t stack_pointer;
} FLASH_SYSCALL;

typedef struct __SWDMemory_TypeDef_Struct
{
	uint32_t  init;
	uint32_t  uninit;
	uint32_t  erase_chip;
	uint32_t  erase_sector;
	uint32_t  program_page;
	FLASH_SYSCALL sys_call_s;
	uint32_t  program_buffer;
	uint32_t  algo_start;
	uint32_t  algo_size;
	uint32_t *algo_blob;
	uint32_t  program_buffer_size;
}SWDMemory_TypeDef_Struct;

/** 
  * @brief  SWD Type structure definition
  */

typedef struct __SWD_TypeDef_Struct
{
	  SWDHardware_TypeDef_Struct  SWD_Hardware;
	  SWDMemory_TypeDef_Struct    SWD_OperatingParameter;
}SWD_TypeDef_Struct;

/** 
  * @brief  SWD Init structure definition  
  */

typedef struct __SWD_InitTypeDef_Struct
{
	uint32_t  init;
	uint32_t  uninit;
	uint32_t  erase_chip;
	uint32_t  erase_sector;
	uint32_t  program_page;
	FLASH_SYSCALL sys_call_s;
	uint32_t  program_buffer;
	uint32_t  algo_start;
	uint32_t  algo_size;
	uint32_t *algo_blob;
	uint32_t  program_buffer_size;
}SWD_InitTypeDef_Struct;

typedef enum
{
    RESET_HOLD,              // Hold target in reset
    RESET_PROGRAM,           // Reset target and setup for flash programming.
    RESET_RUN,               // Reset target and run normally
    RESET_RUN_WITH_DEBUG,    // Reset target and run with debug enabled (required for semihost)
    NO_DEBUG,                // Disable debug on running target
    DEBUG,                 // Enable debug on running target
		HALT,                    // Halt the target without resetting it
    RUN                      // Resume the target without resetting it
} TARGET_RESET_STATE;

/** @defgroup SWD_Exported_Macros
  * @{
  */
#ifndef SWD_GPIO_Pin_NULL
#define SWD_GPIO_Pin_NULL                   ((uint16_t)0x0000)     /* No Pin selected */
#endif
#ifndef SWD_GPIONULL
#define SWD_GPIONULL                       ((uint16_t)0x0000)     /* No GPIO selected */
#endif
/* ------------------------------- SWD_nums_define ---------------------------------- */
/* It is not permitted to modify SWD_NUM_MAX greater than 16 */
#define SWD_NUM_MAX                            2 
/* It is not permitted to modify SWD_NUM_USED greater than SWD_NUM_MAX */
#define SWD_NUM_USED                           2 


#if(SWD_NUM_MAX>2)
#error SWD_NUM_MAX IS GREATER THAN 16, NOT PERMITTED!
#endif

#if(SWD_NOW_NUM>SWD_NUM_MAX)
#error SWD_NUM_USED IS GREATER THAN SWD_NUM_USED, NOT PERMITTED!
#endif

#if(SWD_NUM_USED>=1)
extern SWD_TypeDef_Struct SWD1_BASE;
#define SWD1                       	  (&SWD1_BASE)     /* SWD 1 selected */
#define SWD1_CLKGPIOx                  GPIOD
#define SWD1_CLKGPIO_Pin               GPIO_PIN_10
#define SWD1_DIOGPIOx                  GPIOD
#define SWD1_DIOGPIO_Pin               GPIO_PIN_11
#define SWD1_RSTGPIOx                  GPIOD
#define SWD1_RSTGPIO_Pin               GPIO_PIN_12
#else 
#define SWD1                      		(&SWD1_BASE) 
#define SWD1_CLKGPIOx                  NULL
#define SWD1_CLKGPIO_Pin               NULL
#define SWD1_DIOGPIOx                  NULL
#define SWD1_DIOGPIO_Pin               NULL
#define SWD1_RSTGPIOx                  NULL
#define SWD1_RSTGPIO_Pin               NULL
#endif

#if(SWD_NUM_USED>=2)
extern SWD_TypeDef_Struct SWD2_BASE;
#define SWD2                      		 (&SWD2_BASE)     /* SWD 1 selected */
#define SWD2_CLKGPIOx                  GPIOC
#define SWD2_CLKGPIO_Pin               GPIO_PIN_8
#define SWD2_DIOGPIOx                  GPIOC
#define SWD2_DIOGPIO_Pin               GPIO_PIN_7
#define SWD2_RSTGPIOx                  GPIOE
#define SWD2_RSTGPIO_Pin               GPIO_PIN_12
#else 
#define SWD2                           NULL     /* SWD 1 selected */
#define SWD2_CLKGPIOx                  NULL
#define SWD2_CLKGPIO_Pin               NULL
#define SWD2_DIOGPIOx                  NULL
#define SWD2_DIOGPIO_Pin               NULL
#define SWD2_RSTGPIOx                  NULL
#define SWD2_RSTGPIO_Pin               NULL
#endif

/**
  * @}
  */ 
	
/** @defgroup PWM_Internal_Functions
  * @{
  */
void SWD_SWDIO_SET_OUTPUT(SWD_TypeDef_Struct *SWDx);
void SWD_DIO_SET_INPUT(SWD_TypeDef_Struct *SWDx);
void SWD_WRITE_BIT(SWD_TypeDef_Struct *SWDx,uint32_t bit);
void SWD_CLOCK_CYCLE(SWD_TypeDef_Struct *SWDx);
uint32_t SWD_READ_BIT(SWD_TypeDef_Struct *SWDx);
void SWD_DeInit(SWD_TypeDef_Struct *SWDx);
void SWD_Init(SWD_TypeDef_Struct *SWDx,SWD_InitTypeDef_Struct* SWDx_InitStruct);
void SWD_GPIOConfig(SWD_TypeDef_Struct *SWDx);


//!< API
uint8_t SWJ_ReadDP(SWD_TypeDef_Struct *SWDx,uint8_t adr, uint32_t *val);
uint8_t SWJ_WriteDP(SWD_TypeDef_Struct *SWDx,uint8_t adr, uint32_t val);
uint8_t SWJ_ReadAP(SWD_TypeDef_Struct *SWDx,uint32_t adr, uint32_t *val);
uint8_t SWJ_WriteAP(SWD_TypeDef_Struct *SWDx,uint32_t adr, uint32_t val);
uint8_t SWJ_InitDebug(SWD_TypeDef_Struct *SWDx);
uint8_t SWJ_ReadMem(SWD_TypeDef_Struct *SWDx,uint32_t addr, uint8_t *buf, uint32_t len);
uint8_t SWJ_WriteMem(SWD_TypeDef_Struct *SWDx,uint32_t addr, uint8_t *buf, uint32_t len);
uint8_t SWJ_SetTargetState(SWD_TypeDef_Struct *SWDx,TARGET_RESET_STATE state);
uint8_t swd_flash_syscall_exec(SWD_TypeDef_Struct *SWDx,const FLASH_SYSCALL *sysCallParam, uint32_t entry, uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4);
uint8_t SWJ_WriteMem32(SWD_TypeDef_Struct *SWDx,uint32_t addr, uint32_t val);

#endif


