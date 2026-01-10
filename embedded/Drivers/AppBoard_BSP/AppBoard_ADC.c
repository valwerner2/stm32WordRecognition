#include "AppBoard_ADC.h"

extern ADC_HandleTypeDef hadc1;
extern ADC_HandleTypeDef hadc2;
extern ADC_HandleTypeDef hadc3;

uint32_t readADC(ADC_HandleTypeDef* hadc)
{
	HAL_ADC_Start(hadc);
	HAL_ADC_PollForConversion(hadc, 1000);
	uint32_t u32Value = HAL_ADC_GetValue(hadc);
	HAL_ADC_Stop(hadc);
	return u32Value;
}


uint32_t u32AppBoard_ADC_ReadPoti(void)
{
	return readADC(&hadc3);
}
f32_t f32AppBoard_ADC_ReadPotiDegree(void)
{
	f32_t f32PotiD = u32AppBoard_ADC_ReadPoti();
	return (f32_t)270 / (f32_t)4095 * f32PotiD;
}
uint32_t u32AppBoard_ADC_ReadTemp(void)
{
	return readADC(&hadc2);
}
f32_t f32AppBoard_ADC_ReadTempCelsius(void)
{
	return 25.f + (u32AppBoard_ADC_ReadTemp() / 4096.f * 3.f - 0.75f ) / 0.01f;
}
int16_t s16AppBoard_ADC_ReadMicro(void)
{
	uint32_t adc_value = readADC(&hadc1);
	if (adc_value > 4095) adc_value = 4095;

	uint32_t scaled_value = adc_value << 4;

	int16_t pcm_value = (int16_t)((int32_t)scaled_value - 32768);

	return pcm_value;
}
