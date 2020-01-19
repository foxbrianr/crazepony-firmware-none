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
Led.c file
 Written by: Camel
 Author E-mail: 375836945@qq.com
 Compilation environment: MDK-Lite Version: 4.23
 Initial release time: 2014-01-28
 Features:
 1.Initialization of the led IO ports on the four arms of the aircraft
 2. Initialize all LED lights off by default----
 */

#include "Led.h"
#include "UART1.h"
#include "config.h"
#include "imu.h"
#include "FailSafe.h"

LED_t LEDCtrl;
LEDBuf_t LEDBuf;  // Interface video memory

extern int LostRCFlag;

/********************************************
               Led initialization function
 Features:
 1. Configure the IO output direction of the LED interface
 2. Turn off all LEDs (the default way to boot)
 description:
 Led interface:
 Led1-> PA11
 Led2-> PA8
 Led3-> PB1
 Led4-> PB3
 Corresponding IO = 1, the light is on
 ********************************************/
void LedInit(void) {
   RCC->APB2ENR |= 1 << 2; // Enable PORTA clock
   RCC->APB2ENR |= 1 << 3; // Enable PORTB clock

   RCC->APB2ENR |= 1 << 0; // Enable multiplexed clock
   GPIOB->CRL &= 0XFFFF0F0F; // PB1,3 push-pull output
   GPIOB->CRL |= 0X00003030;
   GPIOB->ODR |= 5 << 1; // PB1,3 pull up

   GPIOA->CRH &= 0XFFFF0FF0; // PA8,11 push-pull output
   GPIOA->CRH |= 0X00003003;
   GPIOA->ODR |= 9 << 0; // PA1,11 pull up

   AFIO->MAPR |= 2 << 24; // Turn off JATG, you must not turn off SWD, otherwise the chip will be invalidated, pro test !!!!!!!!!!!!!!!!! !!!!!!!!
   LedA_off;
   LedB_off;
   LedC_off;
   LedD_off;
}

//Layer Update，10Hz
void LEDReflash(void) {

   if (LEDBuf.bits.A)
      LedA_on;
   else
      LedA_off;

   if (LEDBuf.bits.B)
      LedB_on;
   else
      LedB_off;

   if (LEDBuf.bits.C)
      LedC_on;
   else
      LedC_off;

   if (LEDBuf.bits.D)
      LedD_on;
   else
      LedD_off;
}

void LEDFSM(void) {
   LEDCtrl.event = E_READY;

   if (!imu.ready)
	   // Ready to start imu
      LEDCtrl.event = E_CALI;

   if (1 == LostRCFlag)
      LEDCtrl.event = E_LOST_RC;

   if (!imu.caliPass)
      LEDCtrl.event = E_CALI_FAIL;

   if (Battery.alarm)
      LEDCtrl.event = E_BAT_LOW;

   if (imuCaliFlag)
      LEDCtrl.event = E_CALI;

   if ((Battery.chargeSta))         //battery charge check
      LEDCtrl.event = E_BatChg;

   if (LANDING == altCtrlMode) {
      LEDCtrl.event = E_AUTO_LANDED;
   }

   switch (LEDCtrl.event) {
      case E_READY:
         if (++LEDCtrl.cnt >= 3)      //0 1 2 in loop, 0 on ,else off
            LEDCtrl.cnt = 0;
         if (LEDCtrl.cnt == 0)
            LEDBuf.byte = LA | LB;
         else
            LEDBuf.byte = 0;
         break;
      case E_CALI:
         LEDBuf.byte = LA | LB;
         break;
      case E_BAT_LOW:
         if (++LEDCtrl.cnt >= 3)      //0 1  in loop
            LEDCtrl.cnt = 0;
         if (LEDCtrl.cnt == 0)
            LEDBuf.byte = 0x0f;
         else
            LEDBuf.byte = 0;
         break;
      case E_CALI_FAIL:
         if (++LEDCtrl.cnt >= 4)
            LEDCtrl.cnt = 0;
         if (LEDCtrl.cnt < 2)
            LEDBuf.byte = LC | LD;
         else
            LEDBuf.byte = LA | LB;
         break;
      case E_LOST_RC:
         if (++LEDCtrl.cnt >= 4)
            LEDCtrl.cnt = 0;
         LEDBuf.byte = 1 << LEDCtrl.cnt;
         break;
      case E_AUTO_LANDED:
         LEDBuf.byte = 0x0f;
         break;

      case E_BatChg:
         LEDBuf.byte = 0x00;
         break;

   }

   LEDReflash();
}

