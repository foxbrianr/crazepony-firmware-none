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


#ifndef ALT_H
#define ALT_H

#include "config.h"
#include "MS5611.h"


#define ALT_THREAD_PRD  5000	//ms . store error when sensor updates, but correct on each time step to avoid jumps in estimated value 

extern float alti,Vz,Az;

void inertial_filter_predict(float dt, float x[3]);

void inertial_filter_correct(float e, float dt, float x[3], int i, float w);

void AltitudeCombineThread(void);

#endif

//------------------End of File----------------------------
