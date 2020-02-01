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

#include "config.h"
#include "SysConfig.h"
#include "CommApp.h"
#include "imu.h"
#include "Altitude.h"

uint8_t flyLogF = 0;
uint8_t flyLogApp = 0;
uint32_t lastGetRCTime;
uint8_t appCmdFlag = 0;
uint8_t armState = DISARMED; //,disarmRequest=0;

uint8_t btSrc = 0;

#define MAX_LEN 32
volatile uint8_t UdataBuf[MAX_LEN];
uint8_t bufP = 0; //static
static uint8_t validDataLen = 0;
const uint8_t HEADER[2] =
{ 0xAA, 0x55 };
uint16_t rcData[4] =
{ 1500, 1500, 1500, 1500 };

#define CONSTRAIN(x,min,max) {if(x<min) x=min; if(x>max) x=max;}
extern float dbScaleLinear(float x, float x_end, float deadband);

// Function name: RCDataProcess (void)
// Input: None
// Description: Process the received remote control data.
// The remote control data may come from the 2.4G remote control, such as the function ReceiveDataFormNRF ()
// May also come from a mobile APP remote control, such as the function CommAppCmdProcess ()
// So this function prevents a fault in the CommApp.c file
// This function is executed in the main function at a frequency of 50Hz
// Process the RC data from 2.4G RC or smartphone APP
void RCDataProcess(void)
{

	// The aircraft battery is over-discharged and is in an automatic landing state
	// No longer respond to remote data, use median value
	if (LANDING == altCtrlMode)
	{
		rcData[THROTTLE] = 1500;
		rcData[YAW] = 1500;
		rcData[PITCH] = 1500;
		rcData[ROLL] = 1500;
	}

	CONSTRAIN(rcData[THROTTLE], 1000, 2000);
	CONSTRAIN(rcData[YAW], 1000, 2000);
	CONSTRAIN(rcData[PITCH], 1000, 2000);
	CONSTRAIN(rcData[ROLL], 1000, 2000);

	RC_DATA.THROTTLE = rcData[THROTTLE] - 1000;
	RC_DATA.YAW = YAW_RATE_MAX
			* dbScaleLinear((rcData[YAW] - 1500), 500, APP_YAW_DB);
	RC_DATA.PITCH = Angle_Max
			* dbScaleLinear((rcData[PITCH] - 1500), 500, APP_PR_DB);
	RC_DATA.ROOL = Angle_Max
			* dbScaleLinear((rcData[ROLL] - 1500), 500, APP_PR_DB);

	switch (armState)
	{
	case REQ_ARM:
		if (IMUCheck() && !Battery.alarm)
		{
			armState = ARMED;
			FLY_ENABLE = 0xA5;
		}
		else
		{
			FLY_ENABLE = 0;
			armState = DISARMED;
		}
		break;
	case REQ_DISARM:
		FLY_ENABLE = 0;
		altCtrlMode = MANUAL;	// Added after locking
		zIntReset = 1;
		thrustZSp = 0;
		thrustZInt = estimateHoverThru();
		offLandFlag = 0;

		armState = DISARMED;
		break;
	default:
		break;

	}

}

// Process the MSP command sent to the flight controller
void CommAppCmdProcess(void)
{
	//process
	switch (UdataBuf[4])
	//MSP_SET_4CON
	{
	case MSP_SET_4CON:
		//UdataBuf
		rcData[THROTTLE] = UdataBuf[6] << 8 | UdataBuf[5];
		rcData[YAW] = UdataBuf[8] << 8 | UdataBuf[7];
		rcData[PITCH] = UdataBuf[10] << 8 | UdataBuf[9];
		rcData[ROLL] = UdataBuf[12] << 8 | UdataBuf[11];
		break;
	case MSP_ARM_IT:	//arm，上锁
		armState = REQ_ARM;
		break;
	case MSP_DISARM_IT:
		//disarm
		armState = REQ_DISARM;
		break;
	case MSP_FLY_STATE:
		flyLogApp = 1;
		break;
	case MSP_ACC_CALI:
		imuCaliFlag = 1;
		break;
	case MSP_HEAD_FREE:
		SetHeadFree(1);
		break;
	case MSP_STOP_HEAD_FREE:
		SetHeadFree(0);
		break;
	case MSP_LAND_DOWN:
		// Automatic landing
		altCtrlMode = LANDING;
		break;
	}
}

static uint8_t checksum = 0;

// Process the data stream sent to the flight controller and parse out the MSP command
// The function finally calls CommAppCmdProcess () to process the MSP command
void CommApp(uint8_t ch)
{
	UdataBuf[bufP] = ch;
	if (bufP < 3)
	{
		switch (bufP)
		{
		case 0:
			if (UdataBuf[bufP] == '$')
				bufP++;
			break;
		case 1:
			if (UdataBuf[bufP] == 'M')
				bufP++;
			else
				bufP = 0;
			break;
		case 2:
			if (UdataBuf[bufP] == '<')
				bufP++;
			else
				bufP = 0;
			break;
		}
	}
	else	//valid data
	{
		if (bufP == 3)		//len
		{
			checksum = 0;
			validDataLen = UdataBuf[bufP];
		}

		bufP++;
		if (bufP >= validDataLen + 6)// rec over. process. tobe placed in 50Hz loop
		{
			//chksum
			if (UdataBuf[bufP - 1] == checksum)
			{
				CommAppCmdProcess();		//could be place to main
				btSrc = SRC_APP;
				lastGetRCTime = millis();		//ms
			}
			bufP = 0;
		}
		else
			checksum ^= UdataBuf[bufP - 1];
	}
}

/**
 * @brief send 8bits
 */
static void uart8chk(uint8_t _x)
{
	UartBuf_WD(&UartTxbuf, _x);
	checksum ^= _x;
}
/**
 * @brief send 32bits
 */
static void uart32chk(uint32_t a)
{
	static uint8_t t;
	t = a;
	UartBuf_WD(&UartTxbuf, t);
	checksum ^= t;
	t = a >> 8;
	UartBuf_WD(&UartTxbuf, t);
	checksum ^= t;
	t = a >> 16;
	UartBuf_WD(&UartTxbuf, t);
	checksum ^= t;
	t = a >> 24;
	UartBuf_WD(&UartTxbuf, t);
	checksum ^= t;
}
/**
 * @brief send 16bits
 */
static void uart16chk(int16_t a)
{
	static uint8_t t;
	t = a;
	UartBuf_WD(&UartTxbuf, t);
	checksum ^= t;
	t = a >> 8 & 0xff;
	UartBuf_WD(&UartTxbuf, t);
	checksum ^= t;
}

// Send the basic information of the flight control to the mobile APP,
// mainly including:
// roll, pitch, yaw value, battery level, height information, etc.
// The flight control information will not be sent cyclically,
// but the mobile APP needs to send the MSP status request command MSP_FLY_STATE
void CommAppUpload(void)
{
	uart8chk('$');
	uart8chk('M');
	uart8chk('>');
	checksum = 0;
	uart8chk(12 + 2);
	uart8chk(MSP_FLY_STATE);

	uart16chk((int16_t) (imu.roll * 10));
	uart16chk((int16_t) (imu.pitch * 10));
	uart16chk((int16_t) (imu.yaw * 10));
	uart32chk((int32_t) (-nav.z * 100));	            //altitude
	uart16chk((int16_t) (Battery.BatteryVal * 100));	//battery
	uart16chk((int16_t) (-nav.vz * 1000));

	uart8chk(checksum);

	// ---------------------------------------------
	// Enable serial port sending
	//----------------------------------------------
	USART_ITConfig(USART1, USART_IT_TXE, ENABLE);
}

