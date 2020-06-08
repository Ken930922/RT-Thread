#ifndef __SWD_PIN_H__
#define __SWD_PIN_H__

#include <stdint.h>
#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>

#define SWDIO_PIN    GET_PIN(D, 11)
#define SWCLK_PIN    GET_PIN(D, 10)
#define RESET_PIN    GET_PIN(D, 12)

#define SWCLK_SET()            rt_pin_write(SWCLK_PIN, PIN_HIGH)
#define SWCLK_CLR()            rt_pin_write(SWCLK_PIN, PIN_LOW)
#define RESET_SET()            rt_pin_write(RESET_PIN, PIN_HIGH)
#define RESET_CLR()            rt_pin_write(RESET_PIN, PIN_LOW)
#define SWDIO_SET()            rt_pin_write(SWDIO_PIN, PIN_HIGH)
#define SWDIO_CLR()            rt_pin_write(SWDIO_PIN, PIN_LOW)

#define SWDIO_IN()    				 rt_pin_read(SWDIO_PIN)
#define SWDIO_OUT(n)    			 GPIO_WriteBit(SWDIO_PORT,SWDIO_PIN,n)

#define SWDIO_SET_INPUT()       rt_pin_mode(SWDIO_PIN, PIN_MODE_INPUT_PULLUP)
#define SWDIO_SET_OUTPUT()      rt_pin_mode(SWDIO_PIN, PIN_MODE_OUTPUT)

#define NRF_SWDIO_PIN    GET_PIN(C, 7)
#define NRF_SWCLK_PIN    GET_PIN(C, 8)
#define NRF_RESET_PIN    GET_PIN(E, 12)

#define NRF_SWCLK_SET()            rt_pin_write(NRF_SWCLK_PIN, PIN_HIGH)
#define NRF_SWCLK_CLR()            rt_pin_write(NRF_SWCLK_PIN, PIN_LOW)
#define NRF_RESET_SET()            rt_pin_write(NRF_RESET_PIN, PIN_HIGH)
#define NRF_RESET_CLR()            rt_pin_write(NRF_RESET_PIN, PIN_LOW)
#define NRF_SWDIO_SET()            rt_pin_write(NRF_SWDIO_PIN, PIN_HIGH)
#define NRF_SWDIO_CLR()            rt_pin_write(NRF_SWDIO_PIN, PIN_LOW)

#define NRF_SWDIO_IN()    				 rt_pin_read(NRF_SWDIO_PIN)
#define NRF_SWDIO_OUT(n)    			 GPIO_WriteBit(NRF_SWDIO_PORT,NRF_SWDIO_PIN,n)

#define NRF_SWDIO_SET_INPUT()       rt_pin_mode(NRF_SWDIO_PIN, PIN_MODE_INPUT_PULLUP)
#define NRF_SWDIO_SET_OUTPUT()      rt_pin_mode(NRF_SWDIO_PIN, PIN_MODE_OUTPUT)


/* pin interface */
void SW_PinInit(void);
uint32_t SW_READ_BIT(void);
void SW_CLOCK_CYCLE(void);
void SW_WRITE_BIT(uint32_t bit);
void SW_Pin_DeInit(void);
void DELAY_US(uint32_t us);
void SW_OFF(void);

void NRF_SW_PinInit(void);
uint32_t NRF_SW_READ_BIT(void);
void NRF_SW_CLOCK_CYCLE(void);
void NRF_SW_WRITE_BIT(uint32_t bit);
void NRF_SW_Pin_DeInit(void);
void NRF_SW_OFF(void);
#endif


