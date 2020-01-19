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
 Tim.c file
 Writers: Pony (Camel), Nieyong
 Author E-mail: 375836945@qq.com
 Compiler environment: MDK-Lite Version: 4.23
 First edition Time: 2014-01-28
 Features:
 1. Initialization timer 3 and 4
 2. Timer 3 -> various parameters of a serial printer (manufactured debugging, default serial output data PC)
 3. Timer 4 -> primary flight control loop reference timer, interrupt key part of the timer main priority, and the priority is set to the highest necessary 4
 ------------------------------------
 */
#include "tim.h"
#include "config.h"
#include "imu.h"
#include "control.h"

#define TASK_TICK_FREQ 1000 // Hz primary frequency of the task

uint16_t cntBaro = 0;
uint16_t cntBatChk = 0;

int LedCounter; // LED blinking counter value
float Compass_HMC[3];

uint8_t accUpdated = 0;
volatile uint16_t anyCnt = 0, anyCnt2 = 0;
uint8_t loop500HzFlag, loop200HzFlag, loop50HzFlag, loop600HzFlag,
		loop100HzFlag, loop20HzFlag, loop10HzFlag;
volatile uint16_t loop500Hzcnt, loop200HzCnt, loop50HzCnt, loop600HzCnt,
		loop100HzCnt, loop20HzCnt, loop10HzCnt = 0;

// Entrance Control
void TIM4_IRQHandler(void) // 1ms interrupted once
{
	if (TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET)
	{
		anyCnt++;
		loop200HzCnt++;
		loop100HzCnt++;

		if (++loop50HzCnt * 50 >= (1000))
		{
			loop50HzCnt = 0;
			loop50HzFlag = 1;
		}
		if (++loop20HzCnt * 20 >= 1000)
		{
			loop20HzCnt = 0;
			loop20HzFlag = 1;
		}
		if (++loop10HzCnt * 10 >= 1000)
		{
			loop10HzCnt = 0;
			loop10HzFlag = 1;
		}

		TIM_ClearITPendingBit(TIM4, TIM_FLAG_Update); // Clear interrupt flag
	}
}

int DebugCounter; // print information output time interval count
extern u8 RX_ADDRESS[5];

void TIM3_IRQHandler(void) // interrupt service routine Print
{
	float hoverThru;

	if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)
	{
		Battery.BatteryAD = GetBatteryAD(); // a battery voltage detector
		Battery.BatteryVal = Battery.Bat_K * (Battery.BatteryAD / 4096.0)
				* Battery.ADRef; // actual voltage value calculating
		DebugCounter++;
		if (DebugCounter == 500)
		{
			DebugCounter = 0;
			printf(
					" ******************************************************************\r\n");
			printf(
					" *       ____                      _____                  +---+   *\r\n");
			printf(
					" *      / ___\\                     / __ \\                 | R |   *\r\n");
			printf(
					" *     / /                        / /_/ /                 +---+   *\r\n");
			printf(
					" *    / /   ________  ____  ___  / ____/___  ____  __   __        *\r\n");
			printf(
					" *   / /  / ___/ __ `/_  / / _ \\/ /   / __ \\/ _  \\/ /  / /        *\r\n");
			printf(
					" *  / /__/ /  / /_/ / / /_/  __/ /   / /_/ / / / / /__/ /         *\r\n");
			printf(
					" *  \\___/_/   \\__,_/ /___/\\___/_/    \\___ /_/ /_/____  /          *\r\n");
			printf(
					" *                                                  / /           *\r\n");
			printf(
					" *                                             ____/ /            *\r\n");
			printf(
					" *                                            /_____/             *\r\n");
			printf(
					" ******************************************************************\r\n");
			printf("\r\n");
			printf("\r\n");
			printf(" Yaw ---> %5.2f degree\r\n", (float) imu.yaw);
			printf(" Pitch---> %5.2f degree\r\n", (float) imu.pitch);
			printf(" Roll ---> %5.2f degree\r\n", (float) imu.roll);
			printf("====================================\r\n");
			printf(" Motor M1 PWM---> %d\r\n", TIM2->CCR1);
			printf(" Motor M2 PWM---> %d\r\n", TIM2->CCR2);
			printf(" Motor M3 PWM---> %d\r\n", TIM2->CCR3);
			printf(" Motor M4 PWM---> %d\r\n", TIM2->CCR4);
			printf("====================================\r\n");
			printf(" Pressure ---> %5.2f Pa\r\n", (float) MS5611_Pressure);
			printf(" Altitude ---> %5.2f M\r\n", (float) MS5611_Altitude);
			printf(" Temperature---> %5.2f C\r\n", (float) MS5611_Temperature);
			printf("====================================\r\n");
// The acquired AD value to calculate the actual voltage. After dividing the battery is to AD acquisition hardware, so the result is multiplied by 2
			printf(" Battery Voltage---> %3.2fv\r\n", Battery.BatteryVal);

			hoverThru = estimateHoverThru();
			printf(" Hover Thru---> %3.2f\r\n", hoverThru);
			printf(" RX Addr ---> 0x%x\r\n", RX_ADDRESS[4]);
			printf("====================================\r\n");
		}
		TIM_ClearITPendingBit(TIM3, TIM_FLAG_Update); // Clear interrupt flag
	}
}

// initialize the timer: used to interrupt the process PID
void TIM4_Init(char clock, int Period)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE); // open clock

	TIM_DeInit(TIM4);

	TIM_TimeBaseStructure.TIM_Period = Period;
	TIM_TimeBaseStructure.TIM_Prescaler = clock - 1; // Timing 1ms
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

	TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);
	TIM_ClearFlag(TIM4, TIM_FLAG_Update);

	TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE);
	TIM_Cmd(TIM4, ENABLE);
	printf("Timer 4 init success...\r\n");

}

// initialize the timer 3
void TIM3_Init(char clock, int Period)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE); // open clock

	TIM_DeInit(TIM3);

	TIM_TimeBaseStructure.TIM_Period = Period;
	TIM_TimeBaseStructure.TIM_Prescaler = clock - 1; // Timing 1ms
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
	TIM_ClearFlag(TIM3, TIM_FLAG_Update);

	TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
	TIM_Cmd(TIM3, ENABLE);

	printf("Timer 3 init success...\r\n");
}

void TimerNVIC_Configuration()
{
	NVIC_InitTypeDef NVIC_InitStructure;

	/* NVIC_PriorityGroup 2 */
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	//TIM3
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2; // timer serial printer, a lower priority than attitude solution
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	//TIM4
	NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1; // main loop flight control reference timer, the priority higher than a serial printer
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

}

