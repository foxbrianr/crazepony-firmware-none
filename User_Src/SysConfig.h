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

#ifndef _SYS_CONFIG_H
#define _SYS_CONFIG_H
//
#include "stm32f10x.h"

#define YAW_CORRECT

// Attitude solution uses software to solve,
// no longer uses the hardware solution unit DMP of MPU6050
#define IMU_SW

#define HIGH_FREQ_CTRL

#define NEW_RC

// When the macro is turned on, you can use the serial assistant to print and debug.
// Otherwise use Crazepony host computer

#define UART_DEBUG	1

enum {
	SRC_PC,
	SRC_APP
};
extern uint8_t btSrc;

#endif


//------------------End of File----------------------------
