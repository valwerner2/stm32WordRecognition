//
// Created by valwe on 10/9/25.
//

#include "AppBoard_LEDs.h"

const sPinIDLEDs_t casPinPort[] = {	{LED1_GPIO_Port, LED1_Pin},
									{LED2_GPIO_Port, LED2_Pin},
									{LED3_GPIO_Port, LED3_Pin},
									{LED4_GPIO_Port, LED4_Pin},
									{LED5_GPIO_Port, LED5_Pin},
									{LED6_GPIO_Port, LED6_Pin},
									{LED7_GPIO_Port, LED7_Pin},
									{LED8_GPIO_Port, LED8_Pin}};

void vAppBoard_LED_Write(uint32_t u32LED, uint8_t u8On)
{
	if(--u32LED < 8)
	{
		HAL_GPIO_WritePin(casPinPort[u32LED].psGPIO, casPinPort[u32LED].u16Pin, u8On);
	}
}

void vAppBoard_LEDs_Write(uint32_t u32Val)
{
	for(uint32_t i = 1; i < 9; i++)
	{
		vAppBoard_LED_Write(i, u32Val & 1);
		u32Val = u32Val >> 1;
	}
}

void vAppBoard_LEDs_LEDOn(uint32_t u32LED)
{
	vAppBoard_LED_Write(u32LED, 1);
}

void vAppBoard_LEDs_LEDOff(uint32_t u32LED)
{
	vAppBoard_LED_Write(u32LED, 0);
}
