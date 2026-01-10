#ifndef _APPBOARD_ADC_H_
#define _APPBOARD_ADC_H_

#include "main.h"


uint32_t u32AppBoard_ADC_ReadPoti(void);
f32_t f32AppBoard_ADC_ReadPotiDegree(void);
uint32_t u32AppBoard_ADC_ReadTemp(void);
f32_t f32AppBoard_ADC_ReadTempCelsius(void);
int16_t s16AppBoard_ADC_ReadMicro(void);
float f32AppBoard_ADC_ReadMicro(void);

#endif /* _APPBOARD_ADC_H_ */
