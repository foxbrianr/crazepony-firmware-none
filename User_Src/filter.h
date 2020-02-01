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

#ifndef _FILTER_H
#define _FILTER_H

#include <math.h>

void LPF2pSetCutoffFreq_1(float sample_freq, float cutoff_freq);
float LPF2pApply_1(float sample);

void LPF2pSetCutoffFreq_2(float sample_freq, float cutoff_freq);
float LPF2pApply_2(float sample);

void LPF2pSetCutoffFreq_3(float sample_freq, float cutoff_freq);
float LPF2pApply_3(float sample);

void LPF2pSetCutoffFreq_4(float sample_freq, float cutoff_freq);
float LPF2pApply_4(float sample);

void LPF2pSetCutoffFreq_5(float sample_freq, float cutoff_freq);
float LPF2pApply_5(float sample);

void LPF2pSetCutoffFreq_6(float sample_freq, float cutoff_freq);
float LPF2pApply_6(float sample);

#endif

//------------------End of File----------------------------
