////////////////////////////////////////////////////////////////////////////////////////////////////////
//      ____                      _____                  +---+
//     / ___\                     / __ \                 | R |
//    / /                        / /_/ /                 +---+
//   / /   ________  ____  ___  / ____/___  ____  __   __
//  / /  / ___/ __ `/_  / / _ \/ /   / __ \/ _  \/ /  / /
// / /__/ /  / /_/ / / /_/  __/ /   / /_/ / / / / /__/ /
// \___/_/   \__,_/ /___/\___/_/    \___ /_/ /_/____  /
//                                                 / /
//                                            ____/ /
//                                           /_____/
//
////////////////////////////////////////////////////////////////////////////////////////////////////////
// Writers: Pony (Camel), Xiang (Samit), Nieyong
// Author E-mail: 375836945@qq.com
// Compiler environment: MDK-Lite Version: 5.10
// First edition Time: 2014-01-28
// Features:
// 1. hardware drivers
// 2. Flight Control: self-stabilization, set high, head to the intelligent, automatic landing, fault protection
// 3. Support App and simultaneously control 2401 RC
// 4. App and PC online monitoring, wireless parameter adjustment
///////////////////////////////////////////////////////////////////////////////////////////////////////////


#include "SysConfig.h"
#include "config.h"        
#include "imu.h"
#include "Altitude.h"
#include "CommApp.h"
#include "CommPC.h"
#include "ConfigTable.h"
#include "IMUSO3.h"
#include "control.h"
#include "FailSafe.h"

//sw counter
uint16_t batCnt;
//check executing time and period in different loop

uint32_t startTime[5];
uint32_t execTime[5];
uint32_t realExecPrd[5];	//us , real called period in different loop

/********************************************
 Primary flight control function entry
 Features:
 1. initialize the various hardware
 2. The system initialization parameters
 ********************************************/
int main(void) {

	uint8_t toggle = 1;
	uint32_t toggleCnt = 0;
	// System clock initialization, external crystal HSEs of the clock source 8 * 9 = 72MHz;
	SystemClock_HSE(9);

	cycleCounterInit();	 // Init cycle counter

	// SysTick starts the system tick timer and initializes its interrupt, 1ms
	SysTick_Config(SystemCoreClock / 1000);

	// Serial port 1 initialization
	UART1_init(SysClock, BT_BAUD_Set);

	//	Interrupt initialization
	NVIC_INIT();

	// Internal flash unlock
	STMFLASH_Unlock();

	LoadParamsFromEEPROM();

	// IO initialization
	LedInit();

	// The Bluetooth power supply is initialized and is turned off by default.
	BT_PowerInit();

	// Motor Initialization
	MotorInit();

	// Battery voltage monitoring initialization
	BatteryCheckInit();

	// I2C Initialization
	IIC_Init();

#ifdef IMU_SW
	// Use software to solve
	MPU6050_initialize();
#else

	// Initialize the DMP engine
    MPU6050_DMP_Initialize();
#endif

	// Initialize the magnetometer HMC5883L
	//HMC5883L_SetUp();

	// Initialize NFL24L01 2.4 GH via SPI
	NRF24L01_INIT();

	// Startup waiting
	PowerOn();

	// Bluetooth write configuration
	BT_ATcmdWrite();

	// Initialize Battery Check
	BatteryCheck();

	MS5611_Init();

	IMU_Init();			// sample rate and cutoff freq.
						// sample rate is too low now due to using dmp.

#ifdef UART_DEBUG
	//Timer 3 initialization, serial debugging information output
	TIM3_Init(SysClock, 2000);
#endif

	//Timer 4 is initialized and used for
	// the reference timing of the main loop of the flight control
	TIM4_Init(SysClock, 1000);


	altCtrlMode = MANUAL;

	// Wait for the barometric initialization altitude to complete
	WaitBaroInitOffset();

	delay_ms(1000);
	LedA_on;
	LedB_off;
	LedC_on;
	LedD_off;
	MotorPwmFlash(0, 10, 0, 10);
	delay_ms(1000);

	LedB_on;
	LedC_off;
	LedD_on;
	LedA_off;
	MotorPwmFlash(10, 0, 10, 0);
	delay_ms(1000);


	LedA_on;
	LedB_on;
	LedC_on;
	LedD_on;

	MotorPwmFlash(0, 0, 0, 0);
	delay_ms(1000);

	LedA_off;
	LedB_off;
	LedC_off;
	LedD_off;
	// Flight control control main loop
	while (1) {


#if 1
		/*Use DMP in MPU6050 for imu , it's accurate but slow and time costing and time unstable */
		//special freq for dmp. 1000/7. use 3-5ms if normal

		//if miss time because of other long time task, dmp maybe need to use 10ms
		// Crazepony default solver software
#ifndef IMU_SW
#ifdef DEBUG_NEW_CTRL_PRD
        if(anyCnt>=7)	//take about 3ms, to develop a faster control
#else
        //it will take about 9ms to read. since this prd 5ms is as same as the set outpur rate in dmp.
        //which fit to old control
        if(anyCnt>=5)
#endif
        {
            anyCnt=0;
            realExecPrd[0]=micros()-startTime[0];
            startTime[0]=micros();

DMP_Routing (); // DMP thread all the data update here
DMP_getYawPitchRoll (); // read attitude angle

execTime [0] = micros () - startTime [0]; // measuring task execution time, CPU occupancy rate

        }
#endif

		//100Hz Loop
		//Crazepony uses a control frequency of 100Hz by default
		if (loop100HzCnt >= 10) {
			loop100HzCnt = 0;

			realExecPrd[1] = micros() - startTime[1];
			startTime[1] = micros();

#ifdef IMU_SW
			IMUSO3Thread();
#else
            IMU_Process();
#endif
			accUpdated = 1;

			// barometric pressure reading
			MS5611_ThreadNew();		//FSM, take aboue 0.5ms some time

			// imu calibration
			if (imuCaliFlag) {
				if (IMU_Calibrate()) {
					imuCaliFlag = 0;
					gParamsSaveEEPROMRequset = 1; // requests to the EEPROM
					imu.caliPass = 1;
				}
			}

			CtrlAttiRate();
			CtrlMotor();

			execTime[1] = micros() - startTime[1];
		}

		//Need to recieve 2401 RC instantly so as to clear reg.
		Nrf_Irq();

		//50Hz Loop
		if (loop50HzFlag) {
			loop50HzFlag = 0;
			realExecPrd[3] = micros() - startTime[3];
			startTime[3] = micros();

			RCDataProcess();

			FlightModeFSMSimple();

			if (altCtrlMode == LANDING) {
				AutoLand();
			}

			// high degree of integration
			AltitudeCombineThread();

			CtrlAlti();

			CtrlAttiAng();

			//PC Monitor
#ifndef UART_DEBUG
            if(btSrc!=SRC_APP) {
                //CommPCUploadHandle();	//tobe improved inside
            }
#endif

			execTime[3] = micros() - startTime[3];
		}

		//10Hz loop
		if (loop10HzFlag) {

			// Change the name of Bluetooth to Crazepony
			//Uart1SendaBTCmd(ATcmdNameSet);

			loop10HzFlag = 0;
			realExecPrd[2] = micros() - startTime[2];
			startTime[2] = micros();

			//Check battery every BAT_CHK_PRD ms
			if ((++batCnt) * 100 >= BAT_CHK_PRD) {
				batCnt = 0;
				BatteryCheck();
			}

			// -------------------------------------------------------------------------------------
			// phone APP have requested flight control information is sent to the mobile phone APP
			// -------------------------------------------------------------------------------------
			if (flyLogApp) {
				CommAppUpload();
				flyLogApp = 0;
			}

			//EEPROM Config Table request to write.
			if (gParamsSaveEEPROMRequset) {
				gParamsSaveEEPROMRequset = 0;
				SaveParamsToEEPROM();
			}

			// out of control protection, such as rollover, loss of remote control signals, etc.
			FailSafe();

			// Handle LED flashes event
			LEDFSM();

			execTime[2] = micros() - startTime[2];
		}
#endif

#if 0
		{
			if (toggleCnt++ > 200000) {
				(!toggle) ? LedB_on : LedB_off;
				(toggle) ? LedD_on : LedD_off;
				toggle = !toggle;
				toggleCnt = 0;
			}
		}
#endif

		//pc cmd process. need to return as quickly as ps
#ifndef UART_DEBUG
        if(pcCmdFlag)
        {
            pcCmdFlag=0;
            CommPCProcessCmd();
        }
#endif
        printf(".");

	}	//end of while(1)
}

