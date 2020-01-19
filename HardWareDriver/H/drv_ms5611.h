#ifndef __MS5611_H_
#define __MS5611_H_


#include "stdint.h"

#include "stdbool.h"



typedef void (* baroOpFuncPtr)(void);                       // baro start operation
typedef void (* baroCalculateFuncPtr)(int32_t *pressure, int32_t *temperature);             // baro calculation (filled params are pressure and temperature)


typedef struct
{
    uint16_t ut_delay;
    uint16_t up_delay;
    baroOpFuncPtr start_ut;
    baroOpFuncPtr get_ut;
    baroOpFuncPtr start_up;
    baroOpFuncPtr get_up;
    baroCalculateFuncPtr calculate;
} baro_t;

bool ms5611Detect(baro_t *baro);


#endif


//------------------End of File----------------------------
