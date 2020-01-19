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
 moto.c file
 Writers: Pony (Camel)
 Author E-mail: 375836945@qq.com
 Compiler environment: MDK-Lite Version: 4.23
 First edition Time: 2014-01-28
 Features:
 1. aircraft four PWM motor drive initialization
 2. Mode PWM output timer 2
 ------------------------------------
 */
#include "stm32f10x.h"
#include "Moto.h"
#include "UART1.h"
#include "stdio.h"
int16_t MOTO1_PWM = 0;
int16_t MOTO2_PWM = 0;
int16_t MOTO3_PWM = 0;
int16_t MOTO4_PWM = 0;
/***********************************************
 Function name: MotorPwmFlash (int16_t MOTO1_PWM, int16_t MOTO2_PWM, int16_t MOTO3_PWM, int16_t MOTO4_PWM)
 Function: Update four PWM value
 Input parameters: MOTO1_PWM, MOTO2_PWM, MOTO3_PWM, MOTO4_PWM
 Output: None
 Description: Quad 2 output from the PWM timer, the input range 0-999
 Remarks:
 ***********************************************/
void MotorPwmFlash(int16_t MOTO1_PWM, int16_t MOTO2_PWM, int16_t MOTO3_PWM,
		int16_t MOTO4_PWM)
{
	if (MOTO1_PWM >= Moto_PwmMax)
		MOTO1_PWM = Moto_PwmMax;
	if (MOTO2_PWM >= Moto_PwmMax)
		MOTO2_PWM = Moto_PwmMax;
	if (MOTO3_PWM >= Moto_PwmMax)
		MOTO3_PWM = Moto_PwmMax;
	if (MOTO4_PWM >= Moto_PwmMax)
		MOTO4_PWM = Moto_PwmMax;
	if (MOTO1_PWM <= 0)
		MOTO1_PWM = 0;
	if (MOTO2_PWM <= 0)
		MOTO2_PWM = 0;
	if (MOTO3_PWM <= 0)
		MOTO3_PWM = 0;
	if (MOTO4_PWM <= 0)
		MOTO4_PWM = 0; // 0 defined input can not be less than, greater than 999

	TIM2->CCR1 = MOTO1_PWM;
	TIM2->CCR2 = MOTO2_PWM;
	TIM2->CCR3 = MOTO3_PWM;
	TIM2->CCR4 = MOTO4_PWM; // timer register assigned to
}

/***********************************************
 Function name: MotorInit (void)
 Function: PWM output initializes the timer 2
 Input parameters: None
 Output: None
 Description: This function call, i.e., initializes the timer 2 to PWM output mode
 Remarks:
 ***********************************************/
void MotorInit(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	TIM_OCInitTypeDef TIM_OCInitStructure;
	uint16_t PrescalerValue = 0; // motor control PWM frequency

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE); // open multiplexing clock and the clock of peripheral A
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE); // open 2 clock timer

// set GPIO function.
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2
			| GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

// reset the timer.
	TIM_DeInit(TIM2);

// Configure timer.
	PrescalerValue = (uint16_t) (SystemCoreClock / 24000000) - 1;

	TIM_TimeBaseStructure.TIM_Period = 999;		            //计数上线
	TIM_TimeBaseStructure.TIM_Prescaler = PrescalerValue;	//pwm时钟分频
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; //向上计数

	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

// PWM output mode configured TIM2
	TIM_OCStructInit(&TIM_OCInitStructure);
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_Pulse = 0;    //0
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;

	TIM_OC1Init(TIM2, &TIM_OCInitStructure);
	TIM_OC2Init(TIM2, &TIM_OCInitStructure);
	TIM_OC3Init(TIM2, &TIM_OCInitStructure);
	TIM_OC4Init(TIM2, &TIM_OCInitStructure);

	TIM_OC1PreloadConfig(TIM2, TIM_OCPreload_Enable);
	TIM_OC2PreloadConfig(TIM2, TIM_OCPreload_Enable);
	TIM_OC3PreloadConfig(TIM2, TIM_OCPreload_Enable);
	TIM_OC4PreloadConfig(TIM2, TIM_OCPreload_Enable);

// start the timer.
	TIM_Cmd(TIM2, ENABLE);
	printf("Motor init success...\r\n");
}
