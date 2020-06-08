/**
  ******************************************************************************
  * @file  stm32f40x_swd.C
  * @author  Ken
  * @version  V1.0.0
  * @date  2020-4-28
  * @brief  This file contains all the functions prototypes for the SWD 
  *         firmware library.
  ******************************************************************************
  **/ 

#include "stm32f40x_swd.h"
#include "debug_cm.h"
#define DHCSR 0xE000EDF0 //调试停机控制及状态寄存器    R/W
#define DCRSR 0xE000EDF4 //调试内核寄存器选择者寄存器   W
#define DCRDR 0xE000EDF8 //调试内核寄存器数据寄存器    R/W
#define DEMCR 0xE000EDFC//调试及监视器控制寄存器       R/W
#define MAX_TIMEOUT 1
#define REGWnR (1 << 16)

/* --------------------------------------SWD_BASE--------------------------------- */
SWD_TypeDef_Struct SWD1_BASE, SWD2_BASE;

static void DELAY_US(uint32_t us)
{
    volatile uint32_t i, j;

    for(i = 0; i < us; i++)
    {
        for(j = 0; j < 30U; j++)
        {
            __NOP();
        }
    }
}
/**
  * @brief  Initializes the IO hardware of SWDx.
  * @param SWDx: where x can be  1 to SWD_NUM_USED to select the SWD.
  *        SWD_NUM_USED is defined at stm32f10x_SWD.h, can be 1 to SWD_NUM_MAX, 
	*        and SWD_NUM_MAX is also defined at stm32f10x_SWD.h, can be 1 to 16.
  * @retval :  ERROR or SUCCESS.
  */
void SWD_GPIOConfig(SWD_TypeDef_Struct *SWDx)
{
    GPIO_InitTypeDef GPIO_InitStructure;

	  if(SWDx== NULL)  
			return;	
	
	  /* SWD GPIO Configuration */ 
    if((SWDx->SWD_Hardware.CLK_GPIOx==SWD_GPIONULL)&&  \
			(SWDx->SWD_Hardware.DIO_GPIOx==SWD_GPIONULL) &&  \
			(SWDx->SWD_Hardware.RST_GPIOx==SWD_GPIONULL))		
			return;
	  if((SWDx->SWD_Hardware.CLK_GPIO_Pin==SWD_GPIO_Pin_NULL)&& 	\
			(SWDx->SWD_Hardware.DIO_GPIO_Pin==SWD_GPIO_Pin_NULL) &&  \
			(SWDx->SWD_Hardware.RST_GPIO_Pin==SWD_GPIO_Pin_NULL))		
			return;
	
	  GPIO_InitStructure.Pin = SWDx->SWD_Hardware.CLK_GPIO_Pin;
		GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
		GPIO_InitStructure.Pull = GPIO_NOPULL;
		GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(SWDx->SWD_Hardware.CLK_GPIOx, &GPIO_InitStructure);

		GPIO_InitStructure.Pin = SWDx->SWD_Hardware.DIO_GPIO_Pin;
		GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
		GPIO_InitStructure.Pull = GPIO_NOPULL;
		GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(SWDx->SWD_Hardware.DIO_GPIOx, &GPIO_InitStructure);
		 
		GPIO_InitStructure.Pin = SWDx->SWD_Hardware.RST_GPIO_Pin;
		GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
		GPIO_InitStructure.Pull = GPIO_NOPULL;
		GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(SWDx->SWD_Hardware.RST_GPIOx, &GPIO_InitStructure);
}


/**
  * @brief  swd初始化 硬件
  * @param 	SWDx.
  * @retval :void.
  */
void SWD_Init(SWD_TypeDef_Struct *SWDx,SWD_InitTypeDef_Struct* SWDx_InitStruct)
{
	if(SWDx== NULL)  
		return;	
	if(SWDx == SWD1)
	{
		SWD1_BASE.SWD_Hardware.CLK_GPIOx = SWD1_CLKGPIOx;
		SWD1_BASE.SWD_Hardware.DIO_GPIOx = SWD1_DIOGPIOx;
		SWD1_BASE.SWD_Hardware.RST_GPIOx = SWD1_RSTGPIOx;
		SWD1_BASE.SWD_Hardware.CLK_GPIO_Pin = SWD1_CLKGPIO_Pin;
		SWD1_BASE.SWD_Hardware.DIO_GPIO_Pin = SWD1_DIOGPIO_Pin;
		SWD1_BASE.SWD_Hardware.RST_GPIO_Pin = SWD1_RSTGPIO_Pin;
	}
	else if(SWDx==SWD2)
	{
	  SWD2_BASE.SWD_Hardware.CLK_GPIOx = SWD2_CLKGPIOx;
		SWD2_BASE.SWD_Hardware.DIO_GPIOx = SWD2_DIOGPIOx;
		SWD2_BASE.SWD_Hardware.RST_GPIOx = SWD2_RSTGPIOx;
		SWD2_BASE.SWD_Hardware.CLK_GPIO_Pin = SWD2_CLKGPIO_Pin;
		SWD2_BASE.SWD_Hardware.DIO_GPIO_Pin = SWD2_DIOGPIO_Pin;
		SWD2_BASE.SWD_Hardware.RST_GPIO_Pin = SWD2_RSTGPIO_Pin;
	}	
	SWD_GPIOConfig(SWDx);
	HAL_GPIO_WritePin(SWDx->SWD_Hardware.RST_GPIOx,SWDx->SWD_Hardware.RST_GPIO_Pin,GPIO_PIN_RESET);
	HAL_GPIO_WritePin(SWDx->SWD_Hardware.DIO_GPIOx,SWDx->SWD_Hardware.DIO_GPIO_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(SWDx->SWD_Hardware.CLK_GPIOx,SWDx->SWD_Hardware.CLK_GPIO_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(SWDx->SWD_Hardware.RST_GPIOx,SWDx->SWD_Hardware.RST_GPIO_Pin, GPIO_PIN_SET);
	
	SWDx->SWD_OperatingParameter.init = SWDx_InitStruct->init;
	SWDx->SWD_OperatingParameter.uninit = SWDx_InitStruct->uninit;
	SWDx->SWD_OperatingParameter.erase_chip = SWDx_InitStruct->erase_chip;
	SWDx->SWD_OperatingParameter.erase_sector = SWDx_InitStruct->erase_sector;
	SWDx->SWD_OperatingParameter.program_page = SWDx_InitStruct->program_page;
	SWDx->SWD_OperatingParameter.sys_call_s = SWDx_InitStruct->sys_call_s;
	SWDx->SWD_OperatingParameter.program_buffer = SWDx_InitStruct->program_buffer;
	SWDx->SWD_OperatingParameter.algo_start = SWDx_InitStruct->algo_start;
	SWDx->SWD_OperatingParameter.algo_size = SWDx_InitStruct->algo_size;
	SWDx->SWD_OperatingParameter.algo_blob = SWDx_InitStruct->algo_blob;
	SWDx->SWD_OperatingParameter.program_buffer_size = SWDx_InitStruct->program_buffer_size;
}
/**
  * @brief  恢复默认状态
  * @param 	SWDx.
  * @retval :void.
  */
void SWD_DeInit(SWD_TypeDef_Struct *SWDx)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	if(SWDx== NULL)  
		return;	

	if(SWDx == SWD1)
	{
		SWD1_BASE.SWD_Hardware.CLK_GPIOx = SWD1_CLKGPIOx;
		SWD1_BASE.SWD_Hardware.DIO_GPIOx = SWD1_DIOGPIOx;
		SWD1_BASE.SWD_Hardware.RST_GPIOx = SWD1_RSTGPIOx;
		SWD1_BASE.SWD_Hardware.CLK_GPIO_Pin = SWD1_CLKGPIO_Pin;
		SWD1_BASE.SWD_Hardware.DIO_GPIO_Pin = SWD1_DIOGPIO_Pin;
		SWD1_BASE.SWD_Hardware.RST_GPIO_Pin = SWD1_RSTGPIO_Pin;
	}
	else if(SWDx==SWD2)
	{
	  SWD2_BASE.SWD_Hardware.CLK_GPIOx = SWD2_CLKGPIOx;
		SWD2_BASE.SWD_Hardware.DIO_GPIOx = SWD2_DIOGPIOx;
		SWD2_BASE.SWD_Hardware.RST_GPIOx = SWD2_RSTGPIOx;
		SWD2_BASE.SWD_Hardware.CLK_GPIO_Pin = SWD2_CLKGPIO_Pin;
		SWD2_BASE.SWD_Hardware.DIO_GPIO_Pin = SWD2_DIOGPIO_Pin;
		SWD2_BASE.SWD_Hardware.RST_GPIO_Pin = SWD2_RSTGPIO_Pin;
	}	
	
	GPIO_InitStructure.Pin = SWDx->SWD_Hardware.CLK_GPIO_Pin;
	GPIO_InitStructure.Mode = GPIO_MODE_INPUT;
	GPIO_InitStructure.Pull = GPIO_NOPULL;
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	HAL_GPIO_Init(SWDx->SWD_Hardware.CLK_GPIOx, &GPIO_InitStructure);

	GPIO_InitStructure.Pin = SWDx->SWD_Hardware.DIO_GPIO_Pin;
	GPIO_InitStructure.Mode = GPIO_MODE_INPUT;
	GPIO_InitStructure.Pull = GPIO_NOPULL;
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	HAL_GPIO_Init(SWDx->SWD_Hardware.DIO_GPIOx, &GPIO_InitStructure);
	 
	GPIO_InitStructure.Pin = SWDx->SWD_Hardware.RST_GPIO_Pin;
	GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStructure.Pull = GPIO_NOPULL;
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	HAL_GPIO_Init(SWDx->SWD_Hardware.RST_GPIOx, &GPIO_InitStructure);
	
	HAL_GPIO_WritePin(SWDx->SWD_Hardware.RST_GPIOx,SWDx->SWD_Hardware.RST_GPIO_Pin, GPIO_PIN_SET);
}


/**
  * @brief  读DIO 一个bit
  * @param 	SWDx.
  * @retval :1bit.
  */
uint32_t SWD_READ_BIT(SWD_TypeDef_Struct *SWDx)
{
	if(SWDx== NULL)  
		return 0;	
	uint32_t bit;
	HAL_GPIO_WritePin(SWDx->SWD_Hardware.CLK_GPIOx,SWDx->SWD_Hardware.CLK_GPIO_Pin, GPIO_PIN_RESET);
	bit = HAL_GPIO_ReadPin(SWDx->SWD_Hardware.DIO_GPIOx,SWDx->SWD_Hardware.DIO_GPIO_Pin);
	HAL_GPIO_WritePin(SWDx->SWD_Hardware.CLK_GPIOx,SWDx->SWD_Hardware.CLK_GPIO_Pin, GPIO_PIN_SET);
	return bit;
}

/**
  * @brief  执行一个CLK周期
  * @param 	SWDx.
  * @retval :void.
  */
void SWD_CLOCK_CYCLE(SWD_TypeDef_Struct *SWDx)
{
	if(SWDx== NULL)  
			return;	
	HAL_GPIO_WritePin(SWDx->SWD_Hardware.CLK_GPIOx,SWDx->SWD_Hardware.CLK_GPIO_Pin, GPIO_PIN_RESET);
	DELAY_US(1);
	HAL_GPIO_WritePin(SWDx->SWD_Hardware.CLK_GPIOx,SWDx->SWD_Hardware.CLK_GPIO_Pin, GPIO_PIN_SET);
	DELAY_US(1);
}


/**
  * @brief  写DIO一个bit
  * @param 	SWDx.
  * @retval :void.
  */
void SWD_WRITE_BIT(SWD_TypeDef_Struct *SWDx,uint32_t bit)
{
	if(SWDx== NULL)  
			return;	
	if(bit & 0x01)
    HAL_GPIO_WritePin(SWDx->SWD_Hardware.DIO_GPIOx,SWDx->SWD_Hardware.DIO_GPIO_Pin, GPIO_PIN_SET);
	else
		HAL_GPIO_WritePin(SWDx->SWD_Hardware.DIO_GPIOx,SWDx->SWD_Hardware.DIO_GPIO_Pin, GPIO_PIN_RESET);
  SWD_CLOCK_CYCLE(SWDx);
}

/**
  * @brief  切换DIO为输入
  * @param 	SWDx.
  * @retval :void.
  */
void SWD_DIO_SET_INPUT(SWD_TypeDef_Struct *SWDx)
{
	 GPIO_InitTypeDef GPIO_InitStructure;

	  if(SWDx== NULL)  
			return;	
		GPIO_InitStructure.Pin = SWDx->SWD_Hardware.DIO_GPIO_Pin;
		GPIO_InitStructure.Mode = GPIO_MODE_INPUT;
		GPIO_InitStructure.Pull = GPIO_NOPULL;
		GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(SWDx->SWD_Hardware.DIO_GPIOx, &GPIO_InitStructure);
}

/**
  * @brief  切换DIO为输出
  * @param 	SWDx.
  * @retval :void.
  */
void SWD_DIO_SET_OUTPUT(SWD_TypeDef_Struct *SWDx)
{
	 GPIO_InitTypeDef GPIO_InitStructure;

	  if(SWDx== NULL)  
			return;	
		GPIO_InitStructure.Pin = SWDx->SWD_Hardware.DIO_GPIO_Pin;
		GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
		GPIO_InitStructure.Pull = GPIO_NOPULL;
		GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(SWDx->SWD_Hardware.DIO_GPIOx, &GPIO_InitStructure);
}



/**
  * @brief  通信函数
  * @param 	SWDx.
  * @retval :void.
  */
uint8_t SWD_TransferOnce(SWD_TypeDef_Struct *SWDx,uint32_t request, uint32_t *data)
{
    uint32_t ack = 0;
    uint32_t bit;
    uint32_t val;
    uint32_t parity;

    uint32_t n;

    /* Packet Request */
    parity = 0;
    SWD_WRITE_BIT(SWDx,1);         /* Start Bit */
    bit = request & 0x1;
    SWD_WRITE_BIT(SWDx,bit);       /* APnDP Bit */
    parity += bit;
    bit = (request >> 1) & 0x1;
    SWD_WRITE_BIT(SWDx,bit);       /* RnW Bit */
    parity += bit;
    bit = request >> 2;
    SWD_WRITE_BIT(SWDx,bit);       /* A2 Bit */
    parity += bit;
    bit = request >> 3;
    SWD_WRITE_BIT(SWDx,bit);       /* A3 Bit */
    parity += bit;
    SWD_WRITE_BIT(SWDx,parity);    /* Parity Bit */
    SWD_WRITE_BIT(SWDx,0);         /* Stop Bit */
    SWD_WRITE_BIT(SWDx,1);         /* Park Bit */

    SWD_DIO_SET_INPUT(SWDx);

    /* Turnaround */
    for(n = 1; n; n--)
    {
        SWD_CLOCK_CYCLE(SWDx);
    }

    /* Acknowledge response */
    for(n = 0; n < 3; n++)
    {
        bit = SWD_READ_BIT(SWDx);
        ack  |= bit << n;
    }
    switch(ack)
    {
        case DAP_TRANSFER_OK:
            if(request & DAP_TRANSFER_RnW)  /* read data */
            {
                val = 0;
                parity = 0;

                for(n = 32; n; n--)
                {
                    bit = SWD_READ_BIT(SWDx);  /* Read RDATA[0:31] */
                    parity += bit;
                    val >>= 1;
                    val  |= bit << 31;
                }

                bit = SWD_READ_BIT(SWDx);    /* Read Parity */

                if((parity ^ bit) & 1)
                {
                    ack = DAP_TRANSFER_ERROR;
                }

                if(data) *data = val;

                /* Turnaround */
                for(n = 1; n; n--)
                {
                    SWD_CLOCK_CYCLE(SWDx);
                }

                SWD_DIO_SET_OUTPUT(SWDx);//SWDIO_SET_OUTPUT
            }
            else    /* write data */
            {
                /* Turnaround */
                for(n = 1; n; n--)
                {
                    SWD_CLOCK_CYCLE(SWDx);
                }

                SWD_DIO_SET_OUTPUT(SWDx);//SWDIO_SET_OUTPUT
                /* Write data */
                val = *data;
                parity = 0;

                for(n = 32; n; n--)
                {
                    SWD_WRITE_BIT(SWDx,val); /* Write WDATA[0:31] */
                    parity += val;
                    val >>= 1;
                }

                SWD_WRITE_BIT(SWDx,parity);/* Write Parity Bit */
            }

            /* Idle cycles */
            SWD_DIO_SET_OUTPUT(SWDx);
            HAL_GPIO_WritePin(SWDx->SWD_Hardware.DIO_GPIOx,SWDx->SWD_Hardware.DIO_GPIO_Pin, GPIO_PIN_SET);
            for(n = 0; n < 8; n++)
            {
                SWD_WRITE_BIT(SWDx,0);
            }
            return (ack);

        case DAP_TRANSFER_WAIT:
        case DAP_TRANSFER_FAULT:

            /* WAIT or FAULT response */
            if(0 && ((request & DAP_TRANSFER_RnW) != 0))
            {
                for(n = 32 + 1; n; n--)
                {
                    SWD_CLOCK_CYCLE(SWDx);  /* Dummy Read RDATA[0:31] + Parity */
                }
            }

            /* Turnaround */
            for(n = 1; n; n--)
            {
                SWD_CLOCK_CYCLE(SWDx);
            }

            SWD_DIO_SET_OUTPUT(SWDx);

            if(0 && ((request & DAP_TRANSFER_RnW) == 0))
            {
                HAL_GPIO_WritePin(SWDx->SWD_Hardware.CLK_GPIOx,SWDx->SWD_Hardware.CLK_GPIO_Pin, GPIO_PIN_RESET);

                for(n = 32 + 1; n; n--)
                {
                    SWD_CLOCK_CYCLE(SWDx);  /* Dummy Write WDATA[0:31] + Parity */
                }
            }

            HAL_GPIO_WritePin(SWDx->SWD_Hardware.DIO_GPIOx,SWDx->SWD_Hardware.DIO_GPIO_Pin, GPIO_PIN_SET);
            for(n = 0; n < 8; n++)
            {
                SWD_WRITE_BIT(SWDx,0);
            }
            return (ack);

        default:
            break;
    }

    /* Protocol error */
    for(n = 1 + 32 + 1; n; n--)
    {
        SWD_CLOCK_CYCLE(SWDx);      /* Back off data phase */
    }

    SWD_DIO_SET_OUTPUT(SWDx);
    HAL_GPIO_WritePin(SWDx->SWD_Hardware.DIO_GPIOx,SWDx->SWD_Hardware.DIO_GPIO_Pin, GPIO_PIN_SET);
    for(n = 0; n < 8; n++)
    {
        SWD_WRITE_BIT(SWDx,0);
    }
    return (ack);
}


uint8_t SWD_Transfer(SWD_TypeDef_Struct *SWDx,uint32_t request, uint32_t *data)
{
    uint8_t i, ack;

    for(i = 0; i < 100; i++)
    {
        ack = SWD_TransferOnce(SWDx,request, data);
        // if ack != WAIT
        if(ack != DAP_TRANSFER_WAIT)
        {
            return ack;
        }
    }
    return ack;
}

/**
	* @brief  发送
  * @param 	SWDx.
  * @retval :void.
  */
void SWJ_SendData(SWD_TypeDef_Struct *SWDx,uint16_t data)
{
    uint8_t i;

    for(i = 0; i < 16; i++)
    {
        SWD_WRITE_BIT(SWDx,((data & 0x1) == 1) ? (1) : (0));
        data >>= 1;
    }
}

//-----------------------------------------------------------------------------
// SW_ShiftReset
//-----------------------------------------------------------------------------
//
// Puts the SWD into the reset state by clocking 64 times with SWDIO high.
// Leaves SWDIO an output and high.
//
void SWD_ShiftReset(SWD_TypeDef_Struct *SWDx)
{
    uint8_t i;
    // Drive SWDIO high
    SWD_DIO_SET_OUTPUT(SWDx);
    HAL_GPIO_WritePin(SWDx->SWD_Hardware.DIO_GPIOx,SWDx->SWD_Hardware.DIO_GPIO_Pin, GPIO_PIN_SET);
    // Complete 64 SWCLK cycles
    for (i = 56; i != 0; i--)
    {
        SWD_CLOCK_CYCLE(SWDx);
    }
}


//JTAG切换到SWD
//1故障 0正常
uint8_t SWJ_JTAG2SWD(SWD_TypeDef_Struct *SWDx)
{
	uint8_t i;
	
	HAL_GPIO_WritePin(SWDx->SWD_Hardware.DIO_GPIOx,SWDx->SWD_Hardware.DIO_GPIO_Pin, GPIO_PIN_SET);
		
	SWD_ShiftReset(SWDx);

	SWJ_SendData(SWDx,0xE79E);

	SWD_ShiftReset(SWDx);
	
	HAL_GPIO_WritePin(SWDx->SWD_Hardware.DIO_GPIOx,SWDx->SWD_Hardware.DIO_GPIO_Pin, GPIO_PIN_RESET);

	for(i = 0; i < 16; i++)
	{
			SWD_CLOCK_CYCLE(SWDx);
	}
	return 0;
}


//1故障 0正常
uint8_t SWJ_ReadDP(SWD_TypeDef_Struct *SWDx,uint8_t adr, uint32_t *val)
{
    uint32_t tmp_in;
    uint8_t ack;
    uint8_t err;

    tmp_in = SWD_REG_DP | SWD_REG_R | SWD_REG_ADR(adr);
    ack = SWD_Transfer(SWDx,tmp_in, val);

    (ack == DAP_TRANSFER_OK) ? (err = 0) : (err = 1);
    return err;
}

//1故障 0正常
uint8_t SWJ_WriteDP(SWD_TypeDef_Struct *SWDx,uint8_t adr, uint32_t val)
{
    uint32_t req;
    uint8_t ack;
    uint8_t err;

    req = SWD_REG_DP | SWD_REG_W | SWD_REG_ADR(adr);
    ack = SWD_Transfer(SWDx,req, &val);

    (ack == DAP_TRANSFER_OK) ? (err = 0) : (err = 1);
    return err;
}


/* Read access port register. */
//读AP寄存器
//1故障 0正常
uint8_t SWJ_ReadAP(SWD_TypeDef_Struct *SWDx,uint32_t adr, uint32_t *val)
{
    uint8_t tmp_in, ack, err;

    uint32_t apsel = adr & APSEL;
    uint32_t bank_sel = adr & APBANKSEL;

    if(SWJ_WriteDP(SWDx,DP_SELECT, apsel | bank_sel))
    {
        return 1;
    }

    tmp_in = SWD_REG_AP | SWD_REG_R | SWD_REG_ADR(adr);

    /* first dummy read */
    ack = SWD_Transfer(SWDx,tmp_in, val);
    ack = SWD_Transfer(SWDx,tmp_in, val);

    (ack == DAP_TRANSFER_OK) ? (err = 0) : (err = 1);
    return err;
}

//写AP寄存器
//1故障 0正常

uint8_t SWJ_WriteAP(SWD_TypeDef_Struct *SWDx,uint32_t adr, uint32_t val)
{
    uint8_t req, ack, err;
//		uint32_t sas;
    uint32_t apsel = adr & APSEL;
    uint32_t bank_sel = adr & APBANKSEL;

    /* write DP select */
    if(SWJ_WriteDP(SWDx,DP_SELECT, apsel | bank_sel))
    {
			return 1;

    }
    /* write AP data */
    req = SWD_REG_AP | SWD_REG_W | SWD_REG_ADR(adr);
		ack = SWD_Transfer(SWDx,req, &val);

    /* read DP buff */
    req = SWD_REG_DP | SWD_REG_R | SWD_REG_ADR(DP_RDBUFF);
		ack = SWD_Transfer(SWDx,req, NULL);
//    ack = SWD_Transfer(req, &sas);
    (ack == DAP_TRANSFER_OK) ? (err = 0) : (err = 1);

    return err;
}


/* Read 32-bit word from target memory. */
// AP CSW register, base value
#define CSW_VALUE (CSW_RESERVED | CSW_MSTRDBG | CSW_HPROT | CSW_DBGSTAT | CSW_SADDRINC)


// Write target memory.
//往目标内存写数据
//1故障 0正常
static uint8_t SWJ_WriteData(SWD_TypeDef_Struct *SWDx,uint32_t addr, uint32_t data)
{
    static uint8_t req, ack, err;
		uint32_t val;
    SWJ_WriteAP(SWDx,AP_TAR, addr);

    /* write data */
    req = SWD_REG_AP | SWD_REG_W | AP_DRW;
    ack = SWD_Transfer(SWDx,req, &data);

    /* read DP buff */
    req = SWD_REG_DP | SWD_REG_R | SWD_REG_ADR(DP_RDBUFF);
    ack = SWD_Transfer(SWDx,req, &val);
	
	
    (ack == DAP_TRANSFER_OK) ? (err = 0) : (err = 1);
    return err;
}
//往目标内存写数据u8
//1故障 0正常
static uint8_t SWJ_WriteMem8(SWD_TypeDef_Struct *SWDx,uint32_t addr, uint8_t val)
{
    uint32_t tmp;
    uint8_t err;

		if (SWJ_WriteAP(SWDx,AP_CSW, CSW_VALUE | CSW_SIZE8)) 
		{
        return 1;
    }
    tmp = val << ((addr & 0x03) << 3);
    err = SWJ_WriteData(SWDx,addr, tmp);
    return err;
}
//往目标内存写数据u32
//1故障 0正常
uint8_t SWJ_WriteMem32(SWD_TypeDef_Struct *SWDx,uint32_t addr, uint32_t val)
{
    uint8_t err;

    SWJ_WriteAP(SWDx,AP_CSW, CSW_VALUE | CSW_SIZE32);
    err = SWJ_WriteData(SWDx,addr, val);
    return err;
}

// Write 32-bit word aligned values to target memory using address auto-increment.
// size is in bytes.
//
//1故障 0正常
static uint8_t SWJ_WriteBlock(SWD_TypeDef_Struct *SWDx,uint32_t addr, uint8_t *buf, uint32_t len)
{
    uint8_t req;
    uint32_t size_in_words;
    uint32_t i, ack;

    if(len == 0)  
			return 0;

    size_in_words = len / 4;

    if( SWJ_WriteAP(SWDx,AP_CSW, CSW_VALUE | CSW_SIZE32))
    {
				return 1;
    }
		if( SWJ_WriteAP(SWDx,AP_TAR, addr))
    {
        return 1;
    }

    /* DRW write */
    req = SWD_REG_AP | SWD_REG_W | (3 << 2);

    for(i = 0; i < size_in_words; i++)
    {
        if(SWD_Transfer(SWDx,req, (uint32_t *)buf) != DAP_TRANSFER_OK)
        {
            return 1;
        }

        buf += 4;
    }

    /* read DP buff */
    req = SWD_REG_DP | SWD_REG_R | SWD_REG_ADR(DP_RDBUFF);
    ack = SWD_Transfer(SWDx,req, NULL);

    if(ack == DAP_TRANSFER_OK) 
			return 0;
		else
			return 1;
}

// Read target memory.
//往目标内存读数据
//1故障 0正常
static uint8_t SWJ_ReadData(SWD_TypeDef_Struct *SWDx,uint32_t addr, uint32_t *val)
{
    uint8_t req, ack, err;

    SWJ_WriteAP(SWDx,AP_TAR, addr);

    /* read data */
    req = SWD_REG_AP | SWD_REG_R | AP_DRW;
    ack = SWD_Transfer(SWDx,req, val);

    /* dummy read */
    req = SWD_REG_DP | SWD_REG_R | SWD_REG_ADR(DP_RDBUFF);
    ack = SWD_Transfer(SWDx,req, val);

    (ack == DAP_TRANSFER_OK) ? (err = 0) : (err = 1);
    return err;
}
//往目标内存读数据u32
//1故障 0正常
static uint8_t SWJ_ReadMem32(SWD_TypeDef_Struct *SWDx,uint32_t addr, uint32_t *val)
{
    uint8_t err;
    SWJ_WriteAP(SWDx,AP_CSW, CSW_VALUE | CSW_SIZE32);
    err = SWJ_ReadData(SWDx,addr, val);
    return err;
}
//往目标内存读数据u8
//1故障 0正常
static uint8_t SWJ_ReadMem8(SWD_TypeDef_Struct *SWDx,uint32_t addr, uint8_t *val)
{
    uint32_t tmp;
    uint8_t err;

    SWJ_WriteAP(SWDx,AP_CSW, CSW_VALUE | CSW_SIZE8);

    err = SWJ_ReadData(SWDx,addr, &tmp);

    *val = (uint8_t)(tmp >> ((addr & 0x03) << 3));

    return err;
}

// Read 32-bit word aligned values from target memory using address auto-increment.
// size is in bytes.
//往目标内存读数据
//1故障 0正常
static uint8_t SWJ_ReadBlock(SWD_TypeDef_Struct *SWDx,uint32_t addr, uint8_t *buf, uint32_t len)
{
    uint8_t err, req;
    uint32_t size_in_words;
    uint32_t i, ack;

    if(len == 0)  return 0;

    err = 0;
    size_in_words = len / 4;


    err += SWJ_WriteAP(SWDx,AP_CSW, CSW_VALUE | CSW_SIZE32);
    err += SWJ_WriteAP(SWDx,AP_TAR, addr);

    if(err) return err;

    req = SWD_REG_AP | SWD_REG_R | (3 << 2);

    /* dummy read */
    if(SWD_Transfer(SWDx,req, (uint32_t *)buf) != 0x01)
    {
        return 1;
    }

    for(i = 0; i < size_in_words; i++)
    {
        if(SWD_Transfer(SWDx,req, (uint32_t *)buf) != DAP_TRANSFER_OK)
        {
            return 1;
        }

        buf += 4;
    }

    /* read DP buff */
    req = SWD_REG_DP | SWD_REG_R | SWD_REG_ADR(DP_RDBUFF);
    ack = SWD_Transfer(SWDx,req, NULL);

    (ack == DAP_TRANSFER_OK) ? (err = 0) : (err = 1);
    return err;
}

#define TARGET_AUTO_INCREMENT_PAGE_SIZE    (0x400)

// Write unaligned data to target memory.
// size is in bytes.
//往目标内存写数据
//1故障 0正常
uint8_t swd_write_memory(SWD_TypeDef_Struct *SWDx,uint32_t address, uint8_t *data, uint32_t size)
{
    uint32_t n;

    // Write bytes until word aligned
    while((size > 0) && (address & 0x3))
    {
        if (SWJ_WriteMem8(SWDx,address, *data)) 
				{
            return 1;
        }
        address++;
        data++;
        size--;
    }

    // Write word aligned blocks
    while(size > 3)
    {
        // Limit to auto increment page size
        n = TARGET_AUTO_INCREMENT_PAGE_SIZE - (address & (TARGET_AUTO_INCREMENT_PAGE_SIZE - 1));

        if(size < n)
        {
            n = size & 0xFFFFFFFC; // Only count complete words remaining
        }

        if(SWJ_WriteBlock(SWDx,address, data, n))
        {
            return 1;
        }

        address += n;
        data += n;
        size -= n;
    }

    // Write remaining bytes
    while(size > 0)
    {
        SWJ_WriteMem8(SWDx,address, *data);

        address++;
        data++;
        size--;
    }

    return 0;
}

// Read unaligned data from target memory.
// size is in bytes.
//往目标内存读数据
//1故障 0正常
uint8_t swd_read_memory(SWD_TypeDef_Struct *SWDx,uint32_t address, uint8_t *data, uint32_t size)
{
    uint32_t n;

    // Read bytes until word aligned
    while((size > 0) && (address & 0x3))
    {
        SWJ_ReadMem8(SWDx,address, data);
        address++;
        data++;
        size--;
    }

    // Read word aligned blocks
    while(size > 3)
    {
        // Limit to auto increment page size
        n = TARGET_AUTO_INCREMENT_PAGE_SIZE - (address & (TARGET_AUTO_INCREMENT_PAGE_SIZE - 1));

        if(size < n)
        {
            n = size & 0xFFFFFFFC; // Only count complete words remaining
        }

        if(SWJ_ReadBlock(SWDx,address, data, n))
        {
            return 1;
        }

        address += n;
        data += n;
        size -= n;
    }

    // Read remaining bytes
    while(size > 0)
    {
        SWJ_ReadMem8(SWDx,address, data);
        address++;
        data++;
        size--;
    }

    return 0;
}
//往目标内存读数据
//1故障 0正常
uint8_t SWJ_ReadMem(SWD_TypeDef_Struct *SWDx,uint32_t addr, uint8_t *buf, uint32_t len)
{
    return swd_read_memory(SWDx,addr, buf, len);
}
//往目标内存写数据
//1故障 0正常
uint8_t SWJ_WriteMem(SWD_TypeDef_Struct *SWDx,uint32_t addr, uint8_t *buf, uint32_t len)
{
    return swd_write_memory(SWDx,addr, buf, len);
}


//等目标Halt
//1故障 0正常
static uint8_t SWJ_WaitUntilHalted(SWD_TypeDef_Struct *SWDx)
{
    // Wait for target to stop
    uint32_t val, i, timeout = 1000000;

    for(i = 0; i < timeout; i++)
    {

        if(SWJ_ReadMem32(SWDx,DBG_HCSR, &val))
        {
            return 1;
        }

        if(val & S_HALT)
        {
            return 0;
        }
    }

    return DAP_TRANSFER_ERROR;
}

void swd_set_target_reset(SWD_TypeDef_Struct *SWDx,uint8_t asserted)
{
   if(asserted == 0)
	{
		SWJ_WriteMem32(SWDx,(uint32_t)&SCB->AIRCR, ((0x5FA << SCB_AIRCR_VECTKEY_Pos) |(SCB->AIRCR & SCB_AIRCR_PRIGROUP_Msk) | SCB_AIRCR_SYSRESETREQ_Msk));
	}
}

//设置目标复位状态
//1故障 0正常
uint8_t SWJ_SetTargetState(SWD_TypeDef_Struct *SWDx,TARGET_RESET_STATE state)
{
    uint32_t val;
    int8_t ap_retries = 2;
		if (state != RUN)
		{
			SWD_GPIOConfig(SWDx);
    }
		
    switch (state) 
		{
				case RESET_HOLD:
				{
					swd_set_target_reset(SWDx,1);
				}break;
				case RESET_RUN:
				{
					swd_set_target_reset(SWDx,1);
					DELAY_US(20 * 1000);
					swd_set_target_reset(SWDx,0);
					DELAY_US(20 * 1000);
					SWD_DeInit(SWDx);
				}break;
				case RESET_PROGRAM:
				{
					if (SWJ_InitDebug(SWDx)) 
					{
							return 1;
					}

					// Enable debug
					while(SWJ_WriteMem32(SWDx,DBG_HCSR, DBGKEY | C_DEBUGEN)) 
					{
							if( --ap_retries <=0 )
									return 1;
							// Target is in invalid state?
							swd_set_target_reset(SWDx,1);
							DELAY_US(20 * 1000);
							swd_set_target_reset(SWDx,0);
							DELAY_US(20 * 1000);
					}

					// Enable halt on reset
					if (SWJ_WriteMem32(SWDx,DBG_EMCR, VC_CORERESET)) 
					{
							return 1;
					}

					// Reset again
					swd_set_target_reset(SWDx,1);
					DELAY_US(20 * 1000);
					swd_set_target_reset(SWDx,0);
					DELAY_US(20 * 1000);

					do
					{
						if (SWJ_ReadMem32(SWDx,DBG_HCSR, &val)) 
						{
								return 1;
						}
					} while ((val & S_HALT) == 0);

					// Disable halt on reset
					if (SWJ_WriteMem32(SWDx,DBG_EMCR, 0))
					{
							return 1;
					}
				}break;
        case NO_DEBUG:
				{
					if (SWJ_WriteMem32(SWDx,DBG_HCSR, DBGKEY)) 
					{
							return 1;
					}
				}break;
        case DEBUG:
				{
					SWJ_JTAG2SWD(SWDx);
					
					if (SWJ_WriteDP(SWDx,DP_ABORT, STKCMPCLR | STKERRCLR | WDERRCLR | ORUNERRCLR)) 
					{
							return 1;
					}
					// Ensure CTRL/STAT register selected in DPBANKSEL
					if (SWJ_WriteDP(SWDx,DP_SELECT, 0)) {
							return 1;
					}
					// Power up
					if (SWJ_WriteDP(SWDx,DP_CTRL_STAT, CSYSPWRUPREQ | CDBGPWRUPREQ)) {
							return 1;
					}
					// Enable debug
					if (SWJ_WriteMem32(SWDx,DBG_HCSR, DBGKEY | C_DEBUGEN)) 
					{
							return 1;
					}
				}break;
        case HALT:
				{
					if (SWJ_InitDebug(SWDx)) 
					{
							return 1;
					}
					// Enable debug and halt the core (DHCSR <- 0xA05F0003)
					if (SWJ_WriteMem32(SWDx,DBG_HCSR, DBGKEY | C_DEBUGEN | C_HALT))
					{
							return 1;
					}
					// Wait until core is halted
					do {
						if (SWJ_ReadMem32(SWDx,DBG_HCSR, &val))
						{
								return 1;
						}
					} while ((val & S_HALT) == 0);
				}break;

        case RUN:
				{ 
					if (SWJ_WriteMem32(SWDx,DBG_HCSR, DBGKEY)) 
					{
             return 1;
          }
          SWD_DeInit(SWDx);
				}
        default:
            return 1;
    }

    return 0;
}

//写目标核寄存器
//1故障 0正常
static uint8_t SWJ_WriteCoreReg(SWD_TypeDef_Struct *SWDx,uint32_t n, uint32_t val)
{
    int i = 0, timeout = MAX_TIMEOUT;

    SWJ_WriteMem32(SWDx,DCRDR, val);

    SWJ_WriteMem32(SWDx,DCRSR, n | REGWnR);

    // wait for S_REGRDY
    for(i = 0; i < timeout; i++)
    {
        SWJ_ReadMem32(SWDx,DHCSR, &val);

        if(val & S_REGRDY)
        {
            return 0;
        }
    }

    return 1;
}
//读目标核寄存器
//1故障 0正常
static uint8_t SWJ_ReadCoreReg(SWD_TypeDef_Struct *SWDx,uint32_t n, uint32_t *val)
{
    int i = 0, timeout = MAX_TIMEOUT, err;

    if(SWJ_WriteMem32(SWDx,DCRSR, n))
        return 1;

    // wait for S_REGRDY
    for(i = 0; i < timeout; i++)
    {

        if(SWJ_ReadMem32(SWDx,DHCSR, val))
            return 1;

        if(*val & S_REGRDY)
            break;
    }

    if(i == timeout)
        return 1;

    err = SWJ_ReadMem32(SWDx,DCRDR, val);

    return err;
}
//写debug状态
typedef struct
{
    uint32_t r[16];
    uint32_t xpsr;
} DEBUG_STATE;
//1故障 0正常
uint8_t swd_write_debug_state(SWD_TypeDef_Struct *SWDx,DEBUG_STATE *state)
{
    uint32_t i, status;

    if(SWJ_WriteDP(SWDx,DP_SELECT, 0))
		{
       return 1;
    }
    // R0, R1, R2, R3
    for(i = 0; i < 4; i++)
    {
			if( SWJ_WriteCoreReg(SWDx,i, state->r[i]))
			{
				return 1;
			}
    }

    // R9
		if(SWJ_WriteCoreReg(SWDx,9, state->r[9]))
		{
			return 1;
		} 

    // R13, R14, R15
    for(i = 13; i < 16; i++)
    {
      if( SWJ_WriteCoreReg(SWDx,i, state->r[i]))
			{
				return 1;
			}
    }

    // xPSR
    if(SWJ_WriteCoreReg(SWDx,16, state->xpsr))
		{
			return 1;
		} 
		if( SWJ_WriteMem32(SWDx,DBG_HCSR, DBGKEY | C_DEBUGEN))
		{
			return 1;
		} 
    // check status
    
		if( SWJ_ReadDP(SWDx,DP_CTRL_STAT, &status))
		{
			return 1;
		}
		
    if(status & (STICKYERR | WDATAERR))
    {
        return 1;
    }

    return 0;
}

//flash状态回读????????????????????
//1故障 0正常
uint8_t swd_flash_syscall_exec(SWD_TypeDef_Struct *SWDx,const FLASH_SYSCALL *sysCallParam, uint32_t entry, uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4)
{
    DEBUG_STATE state = {{0}, 0};

    // Call flash algorithm function on target and wait for result.
    state.xpsr     = 0x01000000;          // xPSR: T = 1, ISR = 0
    state.r[0]     = arg1;                   // R0: Argument 1
    state.r[1]     = arg2;                   // R1: Argument 2
    state.r[2]     = arg3;                   // R2: Argument 3
    state.r[3]     = arg4;                   // R3: Argument 4
    state.r[9]     = sysCallParam->static_base;    // SB: Static Base
    state.r[13]    = sysCallParam->stack_pointer;  // SP: Stack Pointer
    state.r[14]    = sysCallParam->breakpoint;       // LR: Exit Point
    state.r[15]    = entry;                           // PC: Entry Point

    if(swd_write_debug_state(SWDx,&state))
    {
        return 1;
    }

    if(SWJ_WaitUntilHalted(SWDx))
    {
        return 1;
    }

    if(SWJ_ReadCoreReg(SWDx,0, &state.r[0]))
    {
        return 1;
    }


    // Flash functions return 0 if successful.
    if(state.r[0] != 0)
    {
        return 1;
    }

    return 0;
}

//将设备JTAG/SWD状态初始化
//1故障 0正常
uint8_t SWJ_InitDebug(SWD_TypeDef_Struct *SWDx)
{
		uint8_t i = 0;
    uint8_t timeout = 100;
    uint32_t tmp = 0;
    uint32_t val;
    /*JTAG 转到 SWD*/
		SWJ_JTAG2SWD(SWDx);

    if(SWJ_ReadDP(SWDx,DP_IDCODE, &val))
    {
        return 1;
    }
    //连接成功
		
		if(SWJ_WriteDP(SWDx,DP_ABORT, STKCMPCLR | STKERRCLR | WDERRCLR | ORUNERRCLR))
		{
        return 1;
    }
		/* Ensure CTRL/STAT register selected in DPBANKSEL */
    if (SWJ_WriteDP(SWDx,DP_SELECT, 0)) {
        return 1;
    }
    /* Power ups */
    if(SWJ_WriteDP(SWDx,DP_CTRL_STAT, CSYSPWRUPREQ | CDBGPWRUPREQ))
		{
			return 1;
		}
		/* Ensure CTRL/STAT register selected in DPBANKSEL */
		for (i = 0; i < timeout; i++) 
		{
			if (SWJ_ReadDP(SWDx,DP_CTRL_STAT, &tmp)) 
			{
					return 1;
			}
			if ((tmp & (CDBGPWRUPACK | CSYSPWRUPACK)) == (CDBGPWRUPACK | CSYSPWRUPACK)) 
			{
					// Break from loop if powerup is complete
					break;
			}
		}
    if (i == timeout) 
		{
        // Unable to powerup DP
        return 1;
    }
    if (SWJ_WriteDP(SWDx,DP_CTRL_STAT, CSYSPWRUPREQ | CDBGPWRUPREQ | TRNNORMAL | MASKLANE)) 
		{
        return 1;
    }
		
    return 0;
}

