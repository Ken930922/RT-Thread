//#include "errors.h"
#include <stdbool.h>
#include "NRFSWD.h"
#include "swd.h"
#ifndef SWD_DEBUG
#define SWD_DEBUG       1
#endif

#if (SWD_DEBUG == 1)
#include <stdio.h>
#define SWD_TRACE   printf
#else
#define SWD_TRACE(...)
#endif

#define DHCSR 0xE000EDF0 //����ͣ�����Ƽ�״̬�Ĵ���    R/W
#define DCRSR 0xE000EDF4 //�����ں˼Ĵ���ѡ���߼Ĵ���   W
#define DCRDR 0xE000EDF8 //�����ں˼Ĵ������ݼĴ���    R/W
#define DEMCR 0xE000EDFC//���Լ����������ƼĴ���       R/W
/***     -END-           **/

#define REGWnR (1 << 16)

#define MAX_TIMEOUT 1

typedef struct
{
    uint32_t r[16];
    uint32_t xpsr;
} DEBUG_STATE;

uint8_t NRF_SWD_TransferOnce(uint32_t request, uint32_t *data)
{
    uint32_t ack = 0;
    uint32_t bit;
    uint32_t val;
    uint32_t parity;

    uint32_t n;

    /* Packet Request */
    parity = 0;
    NRF_SW_WRITE_BIT(1);         /* Start Bit */
    bit = request & 0x1;
    NRF_SW_WRITE_BIT(bit);       /* APnDP Bit */
    parity += bit;
    bit = (request >> 1) & 0x1;
    NRF_SW_WRITE_BIT(bit);       /* RnW Bit */
    parity += bit;
    bit = request >> 2;
    NRF_SW_WRITE_BIT(bit);       /* A2 Bit */
    parity += bit;
    bit = request >> 3;
    NRF_SW_WRITE_BIT(bit);       /* A3 Bit */
    parity += bit;
    NRF_SW_WRITE_BIT(parity);    /* Parity Bit */
    NRF_SW_WRITE_BIT(0);         /* Stop Bit */
    NRF_SW_WRITE_BIT(1);         /* Park Bit */

    NRF_SWDIO_SET_INPUT();

    /* Turnaround */
    for(n = 1; n; n--)
    {
        NRF_SW_CLOCK_CYCLE();
    }

    /* Acknowledge response */
    for(n = 0; n < 3; n++)
    {
        bit = NRF_SW_READ_BIT();
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
                    bit = NRF_SW_READ_BIT();  /* Read RDATA[0:31] */
                    parity += bit;
                    val >>= 1;
                    val  |= bit << 31;
                }

                bit = NRF_SW_READ_BIT();    /* Read Parity */

                if((parity ^ bit) & 1)
                {
                    ack = DAP_TRANSFER_ERROR;
                }

                if(data) *data = val;

                /* Turnaround */
                for(n = 1; n; n--)
                {
                    NRF_SW_CLOCK_CYCLE();
                }

                NRF_SWDIO_SET_OUTPUT();//NRF_SWDIO_SET_OUTPUT
            }
            else    /* write data */
            {
                /* Turnaround */
                for(n = 1; n; n--)
                {
                    NRF_SW_CLOCK_CYCLE();
                }

                NRF_SWDIO_SET_OUTPUT();//NRF_SWDIO_SET_OUTPUT
                /* Write data */
                val = *data;
                parity = 0;

                for(n = 32; n; n--)
                {
                    NRF_SW_WRITE_BIT(val); /* Write WDATA[0:31] */
                    parity += val;
                    val >>= 1;
                }

                NRF_SW_WRITE_BIT(parity);/* Write Parity Bit */
            }

            /* Idle cycles */
            NRF_SWDIO_SET_OUTPUT();
            SWDIO_SET();
            for(n = 0; n < 8; n++)
            {
                NRF_SW_WRITE_BIT(0);
            }
            return (ack);

        case DAP_TRANSFER_WAIT:
        case DAP_TRANSFER_FAULT:

            /* WAIT or FAULT response */
            if(0 && ((request & DAP_TRANSFER_RnW) != 0))
            {
                for(n = 32 + 1; n; n--)
                {
                    NRF_SW_CLOCK_CYCLE();  /* Dummy Read RDATA[0:31] + Parity */
                }
            }

            /* Turnaround */
            for(n = 1; n; n--)
            {
                NRF_SW_CLOCK_CYCLE();
            }

            NRF_SWDIO_SET_OUTPUT();

            if(0 && ((request & DAP_TRANSFER_RnW) == 0))
            {
                SWCLK_CLR();

                for(n = 32 + 1; n; n--)
                {
                    NRF_SW_CLOCK_CYCLE();  /* Dummy Write WDATA[0:31] + Parity */
                }
            }

            SWDIO_SET();
            for(n = 0; n < 8; n++)
            {
                NRF_SW_WRITE_BIT(0);
            }
            return (ack);

        default:
            break;
    }

    /* Protocol error */
    for(n = 1 + 32 + 1; n; n--)
    {
        NRF_SW_CLOCK_CYCLE();      /* Back off data phase */
    }

    NRF_SWDIO_SET_OUTPUT();
    SWDIO_SET();
    for(n = 0; n < 8; n++)
    {
        NRF_SW_WRITE_BIT(0);
    }
    return (ack);
}


#define MAX_SWD_RETRY   100
uint8_t NRF_SWD_Transfer(uint32_t request, uint32_t *data)
{
    uint8_t i, ack;

    for(i = 0; i < MAX_SWD_RETRY; i++)
    {
        ack = NRF_SWD_TransferOnce(request, data);

        // if ack != WAIT
        if(ack != DAP_TRANSFER_WAIT)
        {
            return ack;
        }
    }

    return ack;
}

//count ʱ�Ӹ���
//swdio_logic swdio��ʱ״̬
/*
static void SWJ_SendClock(uint32_t count, uint8_t swdio_logic)
{
    while(count--)
    {
        NRF_SW_WRITE_BIT((swdio_logic) ? (1) : (0));
    }
}
*/
static void SWJ_SendData(uint16_t data)
{
    uint8_t i;

    for(i = 0; i < 16; i++)
    {
        NRF_SW_WRITE_BIT(((data & 0x1) == 1) ? (1) : (0));
        data >>= 1;
    }
}

//-----------------------------------------------------------------------------
// NRF_SW_ShiftReset
//-----------------------------------------------------------------------------
//
// Puts the SWD into the reset state by clocking 64 times with SWDIO high.
// Leaves SWDIO an output and high.
//
void NRF_SW_ShiftReset(void)
{
    uint8_t i;

    // Drive SWDIO high
    NRF_SWDIO_SET_OUTPUT();
    SWDIO_SET();

    // Complete 64 SWCLK cycles
    for (i = 56; i != 0; i--)
    {
        NRF_SW_CLOCK_CYCLE();
    }
}

//JTAG�л���SWD
//1���� 0����
uint8_t NRF_SWJ_JTAG2SWD(void)
{
	uint8_t i;
	
	SWDIO_SET();
		
	NRF_SW_ShiftReset();

	SWJ_SendData(0xE79E);

	NRF_SW_ShiftReset();
	
	SWDIO_CLR();

	for(i = 0; i < 16; i++)
	{
			NRF_SW_CLOCK_CYCLE();
	}
	return 0;
}
//1���� 0����
uint8_t NRF_SWJ_ReadDP(uint8_t adr, uint32_t *val)
{
    uint32_t tmp_in;
    uint8_t ack;
    uint8_t err;

    tmp_in = SWD_REG_DP | SWD_REG_R | SWD_REG_ADR(adr);
    ack = NRF_SWD_Transfer(tmp_in, val);

    (ack == DAP_TRANSFER_OK) ? (err = 0) : (err = 1);
    return err;
}
//1���� 0����
uint8_t NRF_SWJ_WriteDP(uint8_t adr, uint32_t val)
{
    uint32_t req;
    uint8_t ack;
    uint8_t err;

    req = SWD_REG_DP | SWD_REG_W | SWD_REG_ADR(adr);
    ack = NRF_SWD_Transfer(req, &val);

    (ack == DAP_TRANSFER_OK) ? (err = 0) : (err = 1);
    return err;
}

/* Read access port register. */
//��AP�Ĵ���
//1���� 0����
uint8_t NRF_SWJ_ReadAP(uint32_t adr, uint32_t *val)
{
    uint8_t tmp_in, ack, err;

    uint32_t apsel = adr & APSEL;
    uint32_t bank_sel = adr & APBANKSEL;

    if(NRF_SWJ_WriteDP(DP_SELECT, apsel | bank_sel))
    {
        return 1;
    }

    tmp_in = SWD_REG_AP | SWD_REG_R | SWD_REG_ADR(adr);

    /* first dummy read */
    ack = NRF_SWD_Transfer(tmp_in, val);
    ack = NRF_SWD_Transfer(tmp_in, val);

    (ack == DAP_TRANSFER_OK) ? (err = 0) : (err = 1);
    return err;
}

//дAP�Ĵ���
//1���� 0����

uint8_t NRF_SWJ_WriteAP(uint32_t adr, uint32_t val)
{
    uint8_t req, ack, err;
//		uint32_t sas;
    uint32_t apsel = adr & APSEL;
    uint32_t bank_sel = adr & APBANKSEL;

    /* write DP select */
    if(NRF_SWJ_WriteDP(DP_SELECT, apsel | bank_sel))
    {
			return 1;

    }
    /* write AP data */
    req = SWD_REG_AP | SWD_REG_W | SWD_REG_ADR(adr);
		ack = NRF_SWD_Transfer(req, &val);

    /* read DP buff */
    req = SWD_REG_DP | SWD_REG_R | SWD_REG_ADR(DP_RDBUFF);
		ack = NRF_SWD_Transfer(req, NULL);
//    ack = NRF_SWD_Transfer(req, &sas);
    (ack == DAP_TRANSFER_OK) ? (err = 0) : (err = 1);

    return err;
}

/* Read 32-bit word from target memory. */
// AP CSW register, base value
#define CSW_VALUE (CSW_RESERVED | CSW_MSTRDBG | CSW_HPROT | CSW_DBGSTAT | CSW_SADDRINC)


// Write target memory.
//��Ŀ���ڴ�д����
//1���� 0����
static uint8_t SWJ_WriteData(uint32_t addr, uint32_t data)
{
    static uint8_t req, ack, err;
		uint32_t val;
    NRF_SWJ_WriteAP(AP_TAR, addr);

    /* write data */
    req = SWD_REG_AP | SWD_REG_W | AP_DRW;
    ack = NRF_SWD_Transfer(req, &data);

    /* read DP buff */
    req = SWD_REG_DP | SWD_REG_R | SWD_REG_ADR(DP_RDBUFF);
    ack = NRF_SWD_Transfer(req, &val);
	
	
    (ack == DAP_TRANSFER_OK) ? (err = 0) : (err = 1);
    return err;
}
//��Ŀ���ڴ�д����u8
//1���� 0����
static uint8_t NRF_SWJ_WriteMem8(uint32_t addr, uint8_t val)
{
    uint32_t tmp;
    uint8_t err;

		if (NRF_SWJ_WriteAP(AP_CSW, CSW_VALUE | CSW_SIZE8)) 
		{
        return 1;
    }
    tmp = val << ((addr & 0x03) << 3);
    err = SWJ_WriteData(addr, tmp);
    return err;
}
//��Ŀ���ڴ�д����u32
//1���� 0����
uint8_t NRF_SWJ_WriteMem32(uint32_t addr, uint32_t val)
{
    uint8_t err;

    NRF_SWJ_WriteAP(AP_CSW, CSW_VALUE | CSW_SIZE32);
    err = SWJ_WriteData(addr, val);
    return err;
}

// Write 32-bit word aligned values to target memory using address auto-increment.
// size is in bytes.
//
//1���� 0����
static uint8_t SWJ_WriteBlock(uint32_t addr, uint8_t *buf, uint32_t len)
{
    uint8_t req;
    uint32_t size_in_words;
    uint32_t i, ack;

    if(len == 0)  
			return 0;

    size_in_words = len / 4;


    if( NRF_SWJ_WriteAP(AP_CSW, CSW_VALUE | CSW_SIZE32))
    {
        SWD_TRACE("AP_CSW, CSW_VALUE | CSW_SIZE32 failed\r\n");
				return 1;
    }
		if( NRF_SWJ_WriteAP(AP_TAR, addr))
    {
        return 1;
    }

    /* DRW write */
    req = SWD_REG_AP | SWD_REG_W | (3 << 2);

    for(i = 0; i < size_in_words; i++)
    {
        if(NRF_SWD_Transfer(req, (uint32_t *)buf) != DAP_TRANSFER_OK)
        {
            return 1;
        }

        buf += 4;
    }

    /* read DP buff */
    req = SWD_REG_DP | SWD_REG_R | SWD_REG_ADR(DP_RDBUFF);
    ack = NRF_SWD_Transfer(req, NULL);

    if(ack == DAP_TRANSFER_OK) 
			return 0;
		else
			return 1;
}

// Read target memory.
//��Ŀ���ڴ������
//1���� 0����
static uint8_t SWJ_ReadData(uint32_t addr, uint32_t *val)
{
    uint8_t req, ack, err;

    NRF_SWJ_WriteAP(AP_TAR, addr);

    /* read data */
    req = SWD_REG_AP | SWD_REG_R | AP_DRW;
    ack = NRF_SWD_Transfer(req, val);

    /* dummy read */
    req = SWD_REG_DP | SWD_REG_R | SWD_REG_ADR(DP_RDBUFF);
    ack = NRF_SWD_Transfer(req, val);

    (ack == DAP_TRANSFER_OK) ? (err = 0) : (err = 1);
    return err;
}
//��Ŀ���ڴ������u32
//1���� 0����
static uint8_t NRF_SWJ_ReadMem32(uint32_t addr, uint32_t *val)
{
    uint8_t err;
    NRF_SWJ_WriteAP(AP_CSW, CSW_VALUE | CSW_SIZE32);
    err = SWJ_ReadData(addr, val);
    return err;
}
//��Ŀ���ڴ������u8
//1���� 0����
static uint8_t NRF_SWJ_ReadMem8(uint32_t addr, uint8_t *val)
{
    uint32_t tmp;
    uint8_t err;

    NRF_SWJ_WriteAP(AP_CSW, CSW_VALUE | CSW_SIZE8);

    err = SWJ_ReadData(addr, &tmp);

    *val = (uint8_t)(tmp >> ((addr & 0x03) << 3));

    return err;
}

// Read 32-bit word aligned values from target memory using address auto-increment.
// size is in bytes.
//��Ŀ���ڴ������
//1���� 0����
static uint8_t SWJ_ReadBlock(uint32_t addr, uint8_t *buf, uint32_t len)
{
    uint8_t err, req;
    uint32_t size_in_words;
    uint32_t i, ack;

    if(len == 0)  return 0;

    err = 0;
    size_in_words = len / 4;


    err += NRF_SWJ_WriteAP(AP_CSW, CSW_VALUE | CSW_SIZE32);
    err += NRF_SWJ_WriteAP(AP_TAR, addr);

    if(err) return err;

    req = SWD_REG_AP | SWD_REG_R | (3 << 2);

    /* dummy read */
    if(NRF_SWD_Transfer(req, (uint32_t *)buf) != 0x01)
    {
        return 1;
    }

    for(i = 0; i < size_in_words; i++)
    {
        if(NRF_SWD_Transfer(req, (uint32_t *)buf) != DAP_TRANSFER_OK)
        {
            return 1;
        }

        buf += 4;
    }

    /* read DP buff */
    req = SWD_REG_DP | SWD_REG_R | SWD_REG_ADR(DP_RDBUFF);
    ack = NRF_SWD_Transfer(req, NULL);

    (ack == DAP_TRANSFER_OK) ? (err = 0) : (err = 1);
    return err;
}

#define TARGET_AUTO_INCREMENT_PAGE_SIZE    (0x400)

// Write unaligned data to target memory.
// size is in bytes.
//��Ŀ���ڴ�д����
//1���� 0����
uint8_t NRF_swd_write_memory(uint32_t address, uint8_t *data, uint32_t size)
{
    uint32_t n;

    // Write bytes until word aligned
    while((size > 0) && (address & 0x3))
    {
        if (NRF_SWJ_WriteMem8(address, *data)) 
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

        if(SWJ_WriteBlock(address, data, n))
        {
            SWD_TRACE("SWJ write block failed\r\n");
            return 1;
        }

        address += n;
        data += n;
        size -= n;
    }

    // Write remaining bytes
    while(size > 0)
    {
        NRF_SWJ_WriteMem8(address, *data);

        address++;
        data++;
        size--;
    }

    return 0;
}

// Read unaligned data from target memory.
// size is in bytes.
//��Ŀ���ڴ������
//1���� 0����
uint8_t NRF_swd_read_memory(uint32_t address, uint8_t *data, uint32_t size)
{
    uint32_t n;

    // Read bytes until word aligned
    while((size > 0) && (address & 0x3))
    {
        NRF_SWJ_ReadMem8(address, data);
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

        if(SWJ_ReadBlock(address, data, n))
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
        NRF_SWJ_ReadMem8(address, data);
        address++;
        data++;
        size--;
    }

    return 0;
}
//��Ŀ���ڴ������
//1���� 0����
uint8_t NRF_SWJ_ReadMem(uint32_t addr, uint8_t *buf, uint32_t len)
{
    return NRF_swd_read_memory(addr, buf, len);
}
//��Ŀ���ڴ�д����
//1���� 0����
uint8_t NRF_SWJ_WriteMem(uint32_t addr, uint8_t *buf, uint32_t len)
{
    return NRF_swd_write_memory(addr, buf, len);
}


//��Ŀ��Halt
//1���� 0����
static uint8_t NRF_SWJ_WaitUntilHalted(void)
{
    // Wait for target to stop
    uint32_t val, i, timeout = 1000000;

    for(i = 0; i < timeout; i++)
    {

        if(NRF_SWJ_ReadMem32(DBG_HCSR, &val))
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

void NRF_swd_set_target_reset(uint8_t asserted)
{
   if(asserted == 0)
	{
		NRF_SWJ_WriteMem32((uint32_t)&SCB->AIRCR, ((0x5FA << SCB_AIRCR_VECTKEY_Pos) |(SCB->AIRCR & SCB_AIRCR_PRIGROUP_Msk) | SCB_AIRCR_SYSRESETREQ_Msk));
	}
}

//����Ŀ�긴λ״̬
//1���� 0����
uint8_t NRF_SWJ_SetTargetState(TARGET_RESET_STATE state)
{
    uint32_t val;
    int8_t ap_retries = 2;
		if (state != RUN)
		{
        SW_PinInit();
    }
		
    switch (state) 
		{
				case RESET_HOLD:
				{
					NRF_swd_set_target_reset(1);
				}break;
				case RESET_RUN:
				{
					NRF_swd_set_target_reset(1);
					DELAY_US(20 * 1000);
					NRF_swd_set_target_reset(0);
					DELAY_US(20 * 1000);
					NRF_SW_OFF();
				}break;
				case RESET_PROGRAM:
				{
					if (NRF_SWJ_InitDebug()) 
					{
							return 1;
					}

					// Enable debug
					while(NRF_SWJ_WriteMem32(DBG_HCSR, DBGKEY | C_DEBUGEN)) 
					{
							if( --ap_retries <=0 )
									return 1;
							// Target is in invalid state?
							NRF_swd_set_target_reset(1);
							DELAY_US(20 * 1000);
							NRF_swd_set_target_reset(0);
							DELAY_US(20 * 1000);
					}

					// Enable halt on reset
					if (NRF_SWJ_WriteMem32(DBG_EMCR, VC_CORERESET)) 
					{
							return 1;
					}

					// Reset again
					NRF_swd_set_target_reset(1);
					DELAY_US(20 * 1000);
					NRF_swd_set_target_reset(0);
					DELAY_US(20 * 1000);

					do
					{
						if (NRF_SWJ_ReadMem32(DBG_HCSR, &val)) 
						{
								return 1;
						}
					} while ((val & S_HALT) == 0);

					// Disable halt on reset
					if (NRF_SWJ_WriteMem32(DBG_EMCR, 0))
					{
							return 1;
					}
				}break;
        case NO_DEBUG:
				{
					if (NRF_SWJ_WriteMem32(DBG_HCSR, DBGKEY)) 
					{
							return 1;
					}
				}break;
        case DEBUG:
				{
					NRF_SWJ_JTAG2SWD();
					
					if (NRF_SWJ_WriteDP(DP_ABORT, STKCMPCLR | STKERRCLR | WDERRCLR | ORUNERRCLR)) 
					{
							return 1;
					}
					// Ensure CTRL/STAT register selected in DPBANKSEL
					if (NRF_SWJ_WriteDP(DP_SELECT, 0)) {
							return 1;
					}
					// Power up
					if (NRF_SWJ_WriteDP(DP_CTRL_STAT, CSYSPWRUPREQ | CDBGPWRUPREQ)) {
							return 1;
					}
					// Enable debug
					if (NRF_SWJ_WriteMem32(DBG_HCSR, DBGKEY | C_DEBUGEN)) 
					{
							return 1;
					}
				}break;
        case HALT:
				{
					if (NRF_SWJ_InitDebug()) 
					{
							return 1;
					}
					// Enable debug and halt the core (DHCSR <- 0xA05F0003)
					if (NRF_SWJ_WriteMem32(DBG_HCSR, DBGKEY | C_DEBUGEN | C_HALT))
					{
							return 1;
					}
					// Wait until core is halted
					do {
						if (NRF_SWJ_ReadMem32(DBG_HCSR, &val))
						{
								return 1;
						}
					} while ((val & S_HALT) == 0);
				}break;

        case RUN:
				{ 
					if (NRF_SWJ_WriteMem32(DBG_HCSR, DBGKEY)) 
					{
             return 1;
          }
          NRF_SW_OFF();
				}
        default:
            return 1;
    }

    return 0;
}

//дĿ��˼Ĵ���
//1���� 0����
static uint8_t SWJ_WriteCoreReg(uint32_t n, uint32_t val)
{
    int i = 0, timeout = MAX_TIMEOUT;

    NRF_SWJ_WriteMem32(DCRDR, val);

    NRF_SWJ_WriteMem32(DCRSR, n | REGWnR);

    // wait for S_REGRDY
    for(i = 0; i < timeout; i++)
    {
        NRF_SWJ_ReadMem32(DHCSR, &val);

        if(val & S_REGRDY)
        {
            return 0;
        }
    }

    return 1;
}
//��Ŀ��˼Ĵ���
//1���� 0����
static uint8_t SWJ_ReadCoreReg(uint32_t n, uint32_t *val)
{
    int i = 0, timeout = MAX_TIMEOUT, err;

    if(NRF_SWJ_WriteMem32(DCRSR, n))
        return 1;

    // wait for S_REGRDY
    for(i = 0; i < timeout; i++)
    {

        if(NRF_SWJ_ReadMem32(DHCSR, val))
            return 1;

        if(*val & S_REGRDY)
            break;
    }

    if(i == timeout)
        return 1;

    err = NRF_SWJ_ReadMem32(DCRDR, val);

    return err;
}
//дdebug״̬
//1���� 0����
uint8_t NRF_swd_write_debug_state(DEBUG_STATE *state)
{
    uint32_t i, status;

    if(NRF_SWJ_WriteDP(DP_SELECT, 0))
		{
       return 1;
    }
    // R0, R1, R2, R3
    for(i = 0; i < 4; i++)
    {
			if( SWJ_WriteCoreReg(i, state->r[i]))
			{
				return 1;
			}
    }

    // R9
		if(SWJ_WriteCoreReg(9, state->r[9]))
		{
			return 1;
		} 

    // R13, R14, R15
    for(i = 13; i < 16; i++)
    {
      if( SWJ_WriteCoreReg(i, state->r[i]))
			{
				return 1;
			}
    }

    // xPSR
    if(SWJ_WriteCoreReg(16, state->xpsr))
		{
			return 1;
		} 
		if( NRF_SWJ_WriteMem32(DBG_HCSR, DBGKEY | C_DEBUGEN))
		{
			return 1;
		} 
    // check status
    
		if( NRF_SWJ_ReadDP(DP_CTRL_STAT, &status))
		{
			return 1;
		}
		
    if(status & (STICKYERR | WDATAERR))
    {
        SWD_TRACE("write debug states failed\r\n");
        return 1;
    }

    return 0;
}

//flash״̬�ض�????????????????????
//1���� 0����
uint8_t NRF_swd_flash_syscall_exec(const FLASH_SYSCALL *sysCallParam, uint32_t entry, uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4)
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

    if(NRF_swd_write_debug_state(&state))
    {
        SWD_TRACE("swd_write_debug_status failed\r\n");
        return 1;
    }

    if(NRF_SWJ_WaitUntilHalted())
    {
        SWD_TRACE("SWJ_WaitUntilHalted failed\r\n");
        return 1;
    }

    if(SWJ_ReadCoreReg(0, &state.r[0]))
    {
        SWD_TRACE("SWJ_ReadCoreReg failed\r\n");
        return 1;
    }


    // Flash functions return 0 if successful.
    if(state.r[0] != 0)
    {
        SWD_TRACE("resutlt failed:0x%X\r\n", state.r[0]);
        return 1;
    }

    return 0;
}

//���豸JTAG/SWD״̬��ʼ��
//1���� 0����
uint8_t NRF_SWJ_InitDebug(void)
{
		uint8_t i = 0;
    uint8_t timeout = 100;
    uint32_t tmp = 0;
    uint32_t val;
    /*JTAG ת�� SWD*/
		NRF_SWJ_JTAG2SWD();

    if(NRF_SWJ_ReadDP(DP_IDCODE, &val))
    {
        return 1;
    }
    //���ӳɹ�
		SWD_TRACE("DP_IDCODE:0x%X\r\n", val);
		
		if(NRF_SWJ_WriteDP(DP_ABORT, STKCMPCLR | STKERRCLR | WDERRCLR | ORUNERRCLR))
		{
        return 1;
    }
		/* Ensure CTRL/STAT register selected in DPBANKSEL */
    if (NRF_SWJ_WriteDP(DP_SELECT, 0)) {
        return 1;
    }
    /* Power ups */
    if(NRF_SWJ_WriteDP(DP_CTRL_STAT, CSYSPWRUPREQ | CDBGPWRUPREQ))
		{
			return 1;
		}
		/* Ensure CTRL/STAT register selected in DPBANKSEL */
		for (i = 0; i < timeout; i++) 
		{
			if (NRF_SWJ_ReadDP(DP_CTRL_STAT, &tmp)) 
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
    if (NRF_SWJ_WriteDP(DP_CTRL_STAT, CSYSPWRUPREQ | CDBGPWRUPREQ | TRNNORMAL | MASKLANE)) 
		{
        return 1;
    }
		
    return 0;
}

void NRF_softdelay(uint32_t us)
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
