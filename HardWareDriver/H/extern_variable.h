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

#ifndef _EXTERN_VARIABLE_H_
#define _EXTERN_VARIABLE_H_
#include "NRF24L01.h"

//
enum
{
	BT = 0, NFRC, PC, APP
};

#define RC_SRC  BT  //BT or NFRC
//#define BT_SRC  PC  //APP or PC
#define BT_SRC_PC      //both ok
#define BT_SRC_APP

extern uint8_t accUpdated;

//system
extern uint8_t SystemReady_OK;          //System initialization completion flag
extern uint8_t FLY_ENABLE;

//Flying switch
extern uint8_t IIC_ERROR_CNT; //iic error counter, each time tim3 interrupt plus 1, set to 0 if the reading is successful
extern uint8_t I2C2_BUSY;
extern volatile uint32_t TIM3_IRQCNT;      //TIM3 interrupt counter
extern volatile uint32_t TIM2_IRQCNT;      //TIM3 interrupt counter
extern volatile uint8_t MPU6050_I2CData_Ready; //mpu6050 read completion flag = 1 indicates completion of reading

//sensor
typedef struct int16_xyz
{
	int16_t X;
	int16_t Y;
	int16_t Z;
} S_INT16_XYZ;

typedef union
{
	int16_t D[3];
	S_INT16_XYZ V;
} U_INT16_XYZ;

//IMU
typedef struct float_xyz
{
	float X;
	float Y;
	float Z;
} S_FLOAT_XYZ;

typedef union
{
	float D[3];
	S_FLOAT_XYZ V;
} U_FLOAT_XYZ;

typedef struct float_angle
{
	float Roll;
	float Pitch;
	float Yaw;
} S_FLOAT_ANGLE;


extern S_FLOAT_XYZ ACC_F, GYRO_F; //When the conversion result ACC unit G, GYRO in degrees / sec
extern S_FLOAT_XYZ GYRO_I[3];    //Gyroscope integration
extern S_FLOAT_XYZ DIF_ACC;    //Differential acceleration
extern S_FLOAT_XYZ EXP_ANGLE;    //The desired angle
extern S_FLOAT_XYZ DIF_ANGLE; //The difference between desired angle and actual angle
extern S_FLOAT_ANGLE Q_ANGLE;    //Quaternion calculated angles
extern S_INT16_XYZ ACC_AVG, GYRO_AVG; //ACC and the gyro average value after the filtering process after the sliding window
extern S_FLOAT_ANGLE Q_ANGLE;

#endif

//------------------End of File----------------------------

