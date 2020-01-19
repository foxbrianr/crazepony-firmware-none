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

#include "FailSafe.h"
#include "SysConfig.h"
#include "config.h"
#include "imu.h"
#include "Altitude.h"
#include "CommApp.h"
#include "CommPC.h"
#include "ConfigTable.h"
#include "IMUSO3.h"
#include "control.h"

uint32_t newTime = 0;

int LostRCFlag = 0;

#define LAND_THRO 	      500
#define LAND_THRO_ALT_VEL 200
#define LOST_RC_TIME_MAX  1000

// ----------------------------------------------------------------------
void FailSafe(void)
{
	uint16_t lostRCTime = 0;

	//Stop the motors when copter crash down and get a huge pitch or roll value
	if (fabs(imu.pitch) > 80 || fabs(imu.roll) > 80)
	{
		MotorPwmFlash(0, 0, 0, 0);
		FLY_ENABLE = 0;
	}

	//disconnected from the RC
	newTime = millis();	//ms
	lostRCTime =
			(newTime > lastGetRCTime) ?
					(newTime - lastGetRCTime) :
					(65536 - lastGetRCTime + newTime);

	if (lostRCTime > LOST_RC_TIME_MAX)
	{
		if (offLandFlag || (0 != FLY_ENABLE))
		{
			// The aircraft has left the ground offLandFlag,
			// or the idle rotation FLY_ENABLE has been turned on
			altCtrlMode = LANDING;
		}

		LostRCFlag = 1;
	}
	else
	{
		LostRCFlag = 0;
	}
}

// ----------------------------------------------------------------------
void AutoLand(void)
{
	static uint32_t landStartTime = 0;
	uint32_t landTime = 0;

	if (offLandFlag)
	{
		if (landStartTime == 0)
			landStartTime = millis();
		landTime = millis() - landStartTime;
		if (landTime > 4000)
		{
			altCtrlMode = MANUAL;
			FLY_ENABLE = 0;
			offLandFlag = 0;
			landStartTime = 0;
		}
	}
	else
	{
		altCtrlMode = MANUAL;
		FLY_ENABLE = 0;

	}
}

// ----------------------------------------------------------------------
//for quad-copter launch and flight mode switch.
// it's raw and simple for climb rate mode now
// TO BE IMPROVED
// ----------------------------------------------------------------------
void FlightModeFSMSimple(void)
{
	if (FLY_ENABLE)
	{
		if (RC_DATA.THROTTLE >= 600)
		{
			if (altCtrlMode != CLIMB_RATE)
			{
				zIntReset = 1;
				thrustZSp = 0;
				altCtrlMode = CLIMB_RATE;
				offLandFlag = 1;
				altLand = -nav.z; //Record altitude at takeoff

				SetHeadFree(1);
			}
		}
		else
		{
			if (altCtrlMode == MANUAL)
			{
				RC_DATA.THROTTLE = SLOW_THRO; //Manual mode standby 200
			}
		}

	}
}
