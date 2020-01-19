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


#ifndef _ReceiveData_H_
#define _ReceiveData_H_
#include "stm32f10x.h"


//RC remote control
typedef struct int16_rcget
{
    float ROOL;
    float PITCH;
    float THROTTLE;
    float YAW;
}RC_GETDATA;


extern RC_GETDATA RC_DATA,RC_DATA_RAW;//RC processed data
extern uint8_t FLY_ENABLE;//Flight Enable 7 / -5 14/15

void ReceiveDataFormNRF(void);
void ReceiveDataFormUART(void);
void Send_PIDToPC(void);
void Send_AtitudeToPC(void);
//extern int Rool_error_init; // if the plane took off to the left bias, Rool_error_init forward towards increasing the modification; rightward bias, Rool_error_init modify increases in the negative direction
//extern int Pitch_error_init; // if the plane took off forward biased, Pitch_error_init modify increases in the negative direction; toward the roar partial, Pitch_error_init forward towards increased modification
void NRFmatching(void);


#endif


//------------------End of File----------------------------
