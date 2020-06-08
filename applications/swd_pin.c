#include <stdint.h>
#include <stdbool.h>
#include "swd_pin.h"

#define UWBEN1_PIN    GET_PIN(D, 14)
#define UWBEN2_PIN    GET_PIN(D, 15)


void SW_Pin_DeInit(void)
{
	rt_pin_mode(UWBEN1_PIN, PIN_MODE_OUTPUT);
	rt_pin_mode(UWBEN2_PIN, PIN_MODE_OUTPUT);
	rt_pin_mode(RESET_PIN,  PIN_MODE_OUTPUT);
	rt_pin_mode(NRF_RESET_PIN,  PIN_MODE_OUTPUT);
//	rt_pin_mode(SWCLK_PIN, PIN_MODE_OUTPUT_OD);
//	rt_pin_mode(SWDIO_PIN, PIN_MODE_OUTPUT_OD);
	
	rt_pin_write(RESET_PIN, PIN_HIGH);
	rt_pin_write(NRF_RESET_PIN, PIN_HIGH);
//	rt_pin_write(UWBEN1_PIN, PIN_HIGH);
//	rt_pin_write(UWBEN2_PIN, PIN_HIGH);
}

void SW_PinInit(uint8_t ch)
{
	if(ch == UWB_SWD)
	{
		SWDIO_SET_OUTPUT(UWB_SWD);
		rt_pin_mode(SWCLK_PIN, PIN_MODE_OUTPUT);
		rt_pin_mode(RESET_PIN, PIN_MODE_OUTPUT);
		rt_pin_write(RESET_PIN, PIN_LOW);
		rt_pin_write(SWDIO_PIN, PIN_HIGH);
		rt_pin_write(SWCLK_PIN, PIN_LOW);
		rt_pin_write(RESET_PIN, PIN_HIGH);
	}
	else if(ch == NRF_SWD)
	{
		SWDIO_SET_OUTPUT(NRF_SWD);
		rt_pin_mode(NRF_SWCLK_PIN, PIN_MODE_OUTPUT);
		rt_pin_mode(NRF_RESET_PIN, PIN_MODE_OUTPUT);
		rt_pin_write(NRF_RESET_PIN, PIN_LOW);
		rt_pin_write(NRF_SWDIO_PIN, PIN_HIGH);
		rt_pin_write(NRF_SWCLK_PIN, PIN_LOW);
		rt_pin_write(NRF_RESET_PIN, PIN_HIGH);
	}
}

void SW_OFF(uint8_t ch)
{
	if(ch == UWB_SWD)
	{
		rt_pin_mode(SWDIO_PIN, PIN_MODE_INPUT);
		rt_pin_mode(SWCLK_PIN, PIN_MODE_INPUT);
	}
	else if(ch == NRF_SWD)
	{
		rt_pin_mode(NRF_SWDIO_PIN, PIN_MODE_INPUT);
		rt_pin_mode(NRF_SWCLK_PIN, PIN_MODE_INPUT);
	}
}


void DELAY_US(uint32_t us)
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

uint32_t SW_READ_BIT(uint8_t ch)
{
    uint32_t bit;
    SWCLK_CLR(ch);
    bit = SWDIO_IN(ch);
    SWCLK_SET(ch);
    return bit;
}

void SWDIO_SET_INPUT(uint8_t ch)
{
	if(ch == UWB_SWD)
	{
		rt_pin_mode(SWDIO_PIN, PIN_MODE_INPUT_PULLUP);
	}
	else if(ch == NRF_SWD)
	{
		rt_pin_mode(NRF_SWDIO_PIN, PIN_MODE_INPUT_PULLUP);
	}
}

void SWDIO_SET_OUTPUT(uint8_t ch)
{
	if(ch == UWB_SWD)
	{
		rt_pin_mode(SWDIO_PIN, PIN_MODE_OUTPUT);
	}
	else if(ch == NRF_SWD)
	{
		rt_pin_mode(NRF_SWDIO_PIN, PIN_MODE_OUTPUT);
	}
}
int SWDIO_IN(uint8_t ch)
{
	if(ch == UWB_SWD)
	{
		return rt_pin_read(SWDIO_PIN);
	}
	else if(ch == NRF_SWD)
	{
		return rt_pin_read(NRF_SWDIO_PIN);
	}
	else
		return 0;
}

void SWDIO_SET(uint8_t ch)
{
	if(ch == UWB_SWD)
	{
		 rt_pin_write(SWDIO_PIN, PIN_HIGH);
	}
	else if(ch == NRF_SWD)
	{
		 rt_pin_write(NRF_SWDIO_PIN, PIN_HIGH);
	}
}

void SWDIO_CLR(uint8_t ch)
{
	if(ch == UWB_SWD)
	{
		 rt_pin_write(SWDIO_PIN, PIN_LOW);
	}
	else if(ch == NRF_SWD)
	{
		 rt_pin_write(NRF_SWDIO_PIN, PIN_LOW);
	}
}

void RESET_SET(uint8_t ch)
{
	if(ch == UWB_SWD)
	{
		 rt_pin_write(RESET_PIN, PIN_HIGH);
	}
	else if(ch == NRF_SWD)
	{
		rt_pin_write(NRF_RESET_PIN, PIN_HIGH);
	}
}

void RESET_CLR(uint8_t ch)
{
	if(ch == UWB_SWD)
	{
		 rt_pin_write(RESET_PIN, PIN_LOW);
	}
	else if(ch == NRF_SWD)
	{
		rt_pin_write(NRF_RESET_PIN, PIN_LOW);
	}
}

void SWCLK_CLR(uint8_t ch)
{
	if(ch == UWB_SWD)
	{
		 rt_pin_write(SWCLK_PIN, PIN_LOW);
	}
	else if(ch == NRF_SWD)
	{
		rt_pin_write(NRF_SWCLK_PIN, PIN_LOW);
	}
}

void SWCLK_SET(uint8_t ch)
{
	if(ch == UWB_SWD)
	{
		 rt_pin_write(SWCLK_PIN, PIN_HIGH);
	}
	else if(ch == NRF_SWD)
	{
		rt_pin_write(NRF_SWCLK_PIN, PIN_HIGH);
	}
}


void SW_CLOCK_CYCLE(uint8_t ch)
{
    SWCLK_CLR(ch);
    DELAY_US(1);///PIN_DELAY();//此处以后增加频率关系
    SWCLK_SET(ch);
		DELAY_US(1);
}

void SW_WRITE_BIT(uint8_t ch,uint32_t bit)
{
	if(bit & 0x01)
		SWDIO_SET(ch);
	else
		SWDIO_CLR(ch);
	SW_CLOCK_CYCLE(ch);
}
