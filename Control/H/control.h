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


#ifndef _CONTROL_H_
#define _CONTROL_H_
#include "stm32f10x.h"


//#define Debug // Debugging conditional compilation or not

//Rotation speed to be
// wait speed
#define SLOW_THRO 200

//The maximum angle of inclination of the plane defined
#define  Angle_Max  40.0

#define  YAW_RATE_MAX  180.0f/M_PI_F		//deg/s  

// Correct the attitude error, which can be used to resist the
//initial imbalance caused by center of gravity shift, etc.

//#define Rool_error_init 7   // if the plane took off to the left bias, Rool_error_init forward towards increasing the modification; rightward bias, Rool_error_init modify increases in the negative direction
//#define Pitch_error_init -5 // if the plane took off forward biased,   Pitch_error_init modify increases in the negative direction; rearward bias, Pitch_error_init forward towards increased modification
//Given the high part
#define LAND_SPEED						1.2f		//m/s^2
#define ALT_VEL_MAX 					4.0f

enum {CLIMB_RATE=0,MANUAL,LANDING};
extern uint8_t altCtrlMode;
extern float hoverThrust;
extern uint8_t zIntReset;
extern uint8_t offLandFlag;
extern float altLand;
extern uint8_t isAltLimit;
extern float thrustZSp,thrustZInt;

//PID structure
typedef struct
{
    float P;
    float I;
    float D;
    float Desired;
    float Error;
    float PreError;
    float PrePreError;
    float Increment;
    float Integ;
		float iLimit;
    float Deriv;
    float Output;
 
}PID_Typedef;


//Flash write parameter structure
typedef struct
{
  u16 WriteBuf[10];       //Temporary array of write flash
  u16 ReadBuf[10];        //Flash reads the temporary array
  
}Parameter_Typedef;


void Controler(void);
void PID_INIT(void);
void PID_Calculate(void);

void CtrlAttiAng(void);
void CtrlAttiRate(void);
void CtrlAlti(void);
void CtrlAltiVel(void);
void CtrlMotor(void);
void CtrlTest(void);
void CtrlAttiRateNew(void);
void CtrlAttiNew(void);
float estimateHoverThru(void);

void SetHeadFree(uint8_t on);

// write the temporary number written in flash, assigned by NRF24L01_RXDATA [i]
extern u16 PIDWriteBuf[3];

extern PID_Typedef pitch_angle_PID;
extern PID_Typedef pitch_rate_PID;

extern PID_Typedef roll_angle_PID;
extern PID_Typedef roll_rate_PID;

extern PID_Typedef yaw_angle_PID;
extern PID_Typedef yaw_rate_PID;

extern PID_Typedef	alt_PID;
extern PID_Typedef alt_vel_PID;


extern float gyroxGloble;
extern float gyroyGloble;

extern volatile unsigned char motorLock;
#endif

//------------------End of File----------------------------

