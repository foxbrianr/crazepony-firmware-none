/*    
 ____                      _____                  +---+
 / ___\                     / __ \                 | R |
 / /                        / /_/ /                 +---+
 / /   ________  ____  ___  / ____/___  ____  __   __
 / /  / ___/ __ `/_  / / _ \/ /   / __ \/ _  \/ /  / /
 / /__/ /  / /_/ / / /_/  __/ /   / /_/ / / / / /__/ /
 \___/_/   \__,_/ /___/\___/_/    \___ /_/ /_/____  /
 / /
 ____/ /
 /_____/
 */
#include "delay.h"
#include "UART1.h"
#include "config.h"
#include "stm32f10x_it.h"

static u8 fac_us = 0; //us delay times Multiplier
static u16 fac_ms = 0; //ms delay times Multiplier


// initialization delay function
// SYSTICK fixed clock HCLK clock 1/8
// SYSCLK: System Clock
/***********************************************
Function name: delay_init (u8 SYSCLK)
Function: initialization delay function
Input parameters: SYSCLK
Output: None
Description: This delay is due to the timer interrupt count is completed, it is necessary to assign the appropriate registers and the clock
Note: the system clock output parameters
***********************************************/

void delay_init(u8 SYSCLK) {
	SysTick->CTRL &= 0xfffffffb; // bit2 clear, selects the external clock HCLK / 
	fac_us = SYSCLK / 8;
	fac_ms = (u16) fac_us * 1000;
	printf("Delay function init success...\r\n");
}

void delay_ms(uint16_t nms) {
	uint32_t t0 = micros();
	while (micros() - t0 < nms * 1000);

}

void delay_us(u32 nus) {
	uint32_t t0 = micros();
	while (micros() - t0 < nus)	;

}

void Delay(unsigned long delay_time) {
	long i;

	for (i = 0; i < delay_time; i++);

}

//------------------End of File----------------------------
