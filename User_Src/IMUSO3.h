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

#ifndef IMUSO3_H
#define IMUSO3_H


/* Function prototypes */
static float invSqrt(float number);
static void NonlinearSO3AHRSinit(float ax, float ay, float az, float mx, float my, float mz);
static void NonlinearSO3AHRSupdate(float gx, float gy, float gz, float ax, float ay, float az, float mx, float my, float mz, float twoKp, float twoKi, float dt);

void IMUSO3Thread(void);

#endif


//------------------End of File----------------------------
