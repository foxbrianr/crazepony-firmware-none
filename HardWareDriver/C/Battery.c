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

Battery.c file
Written by: Camel
Author E-mail: 375836945@qq.com
Compilation environment: MDK-Lite Version: 4.23
Initial release time: 2014-01-28
Features:
1. Battery detection AD initialization
2. For low voltage detection, provide on-chip temperature sensor drive
------------------------------------
*/

#include "Battery.h"
#include "UART1.h"
#include "stdio.h"
#include "CommApp.h"
#include "ReceiveData.h"
#include "control.h"

//Examples of a voltage information structure
Bat_Typedef Battery;


//Initialization battery testing ADC
//ADC1 open channel 8
//BatteryCheck---->PB0
void BatteryCheckInit()
{
  
 //Before the beginning of the analog input PB0
  RCC->APB2ENR|=1<<3;    //Clock enable port PORTB
  GPIOB->CRL&=0XFFFFFFF0;//PB0 analog input
  //Channel 8
  RCC->APB2ENR|=1<<9;    //ADC1 Clock Enable
  RCC->APB2RSTR|=1<<9;   //ADC1 reset
  RCC->APB2RSTR&=~(1<<9);//Reset End
  RCC->CFGR&=~(3<<14);   //Clear division factor
  //SYSCLK / DIV2 = 12M ADC clock is set to 12M, ADC clock can not exceed the maximum 14M!
  //Otherwise it will cause the ADC accuracy loss!
  RCC->CFGR|=2<<14;
  ADC1->CR1&=0XF0FFFF;   //Work mode is cleared
  ADC1->CR1|=0<<16;      //Stand-alone mode
  ADC1->CR1&=~(1<<8);    //Non-scan mode
  ADC1->CR2&=~(1<<1);    //Single conversion mode
  ADC1->CR2&=~(7<<17);
  ADC1->CR2|=7<<17;       //Conversion software control
  ADC1->CR2|=1<<20;      //Trigger (SWSTART) using an external !!! must be used to trigger an event
  ADC1->CR2&=~(1<<11);   //Align Right
  ADC1->CR2|=1<<23;      //Temperature sensor is enabled

  ADC1->SQR1&=~(0XF<<20);
  ADC1->SQR1&=0<<20;     //A conversion rule sequence is only 1 conversion rule sequence
  //Set the sampling time channel 1
  ADC1->SMPR2&=~(7<<3);  //Clear Channel 1 sampling time
   ADC1->SMPR2|=7<<3;     //Channel 1 239.5 cycle, the sampling time can be increased to improve the accuracy of the

   ADC1->SMPR1&=~(7<<18);  //Clear Channel original settings 16
  ADC1->SMPR1|=7<<18;     //Channel 16 239.5 cycle, the sampling time can be increased to improve the accuracy of the

  ADC1->CR2|=1<<0;     //Open AD converter
  ADC1->CR2|=1<<3;       //Enable Reset calibration
  while(ADC1->CR2&1<<3); //Wait for the end of calibration
  //This bit is set by software cleared by hardware. After the calibration register is initialized bit will be cleared.
  ADC1->CR2|=1<<2;        //Open calibration AD
  while(ADC1->CR2&1<<2);  //Wait for the end of calibration
  //This bit is set by software to start the calibration, and cleared by hardware at the end of calibration
  
  Battery.BatReal = 3.95;//V is modified when the actual battery voltage calibration voltage unit
  Battery.ADinput = 1.98;//Modifying unit is a voltage v R15 and R17 is connected at the calibration voltage
  Battery.ADRef   = 3.26;//Modify microcontroller unit when the supply voltage calibration voltage v
  Battery.Bat_K   = Battery.BatReal/Battery.ADinput;//Calculates a voltage coefficient calculation
  Battery.overDischargeCnt = 0;
  
  printf("Batter voltage AD init ...\r\n");
  
}


// Get ADC value
// ch: channel value 0 ~ 16
// Return value: conversion result
u16 Get_Adc(u8 ch)   
{
  //设置转换序列
  ADC1->SQR3&=0XFFFFFFE0;//Regular sequence the channel ch 1
  ADC1->SQR3|=ch;
  ADC1->CR2|=1<<22;       //启动规则转换通道
  while(!(ADC1->SR&1<<1));//Wait for the end of conversion
  return ADC1->DR;        //Return value adc
}

//获取通道ch的转换值，取times次,然后平均 
//ch: channel number
//times: Get Occurrence
//Return Value: times the average of the results of conversion of the channel ch
u16 Get_Adc_Average(u8 ch,u8 times)
{
  u32 temp_val=0;
  u8 t;
  for(t=0;t<times;t++)
  {
    temp_val+=Get_Adc(ch);
  }
  return temp_val/times;
} 

//得到ADC采样内部温度传感器的温度值
//返回值3位温度值 XXX*0.1C
int Get_Temp(void)
{
  u16 temp_val=0;
  u8 t;
  float temperate;
  for(t=0;t<20;t++)//Read 20 times the average
  {
    temp_val+=Get_Adc(16);//A temperature sensor passage 16
  }
  temp_val/=20;
  temperate=(float)temp_val*(3.3/4096);//To obtain the voltage value of the temperature sensor
  temperate=(1.43-temperate)/0.0043+25;//To calculate the current temperature
  temperate*=10;//Expand tenfold, the use of one decimal place
  return (int)temperate;
}


//Returned to the battery voltage value AD
int GetBatteryAD()
{
 return Get_Adc_Average(8,5);
}

#include "BT.h"

//检测电池电压
void BatteryCheck(void)
{
    Battery.BatteryAD  = GetBatteryAD();            //电池电压检测
    Battery.BatteryVal = Battery.Bat_K * (Battery.BatteryAD/4096.0) * Battery.ADRef;//实际电压 值计算

    if(FLY_ENABLE){
      //处于电机开启等飞行状态，在过放电压值（BAT_OVERDIS_VAL）以上0.03v以上，开始报警
      if(Battery.BatteryVal <= (BAT_OVERDIS_VAL + 0.03)){
          Battery.alarm=1;
      }else{
          Battery.alarm=0;
      }

      //Over-discharge protection, Battery overdischarge protect
      if(Battery.BatteryVal <= BAT_OVERDIS_VAL){
        Battery.overDischargeCnt++;

        if(Battery.overDischargeCnt > 8){
          altCtrlMode=LANDING;
          rcData[0]=1500;rcData[1]=1500;rcData[2]=1500;rcData[3]=1500;
        }
      }else{
        Battery.overDischargeCnt = 0;
      }
    }else{
      if((Battery.BatteryVal < BAT_ALARM_VAL)&&(Battery.BatteryVal > BAT_CHG_VAL)){  //低于3.7v 且大于充电检测电压 BAT_CHG_VAL
        Battery.alarm=1;
      }else{
        Battery.alarm=0;
      }
    }

    if(Battery.BatteryVal < BAT_CHG_VAL){ //on charge
      Battery.chargeSta = 1;
      BT_off();
    }else{
      Battery.chargeSta = 0;
    }

}



