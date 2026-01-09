#include "AppBoard_Buttons.h"

const sPinIDButtons_t casButtons[] = {
		{BUTTON_CENTER_GPIO_Port, 	BUTTON_CENTER_Pin, 	APPBOARD_BUTTON_CENTER},
		{BUTTON_UP_GPIO_Port, 		BUTTON_UP_Pin, 		APPBOARD_BUTTON_UP},
		{BUTTON_RIGHT_GPIO_Port, 	BUTTON_RIGHT_Pin, 	APPBOARD_BUTTON_RIGHT},
		{BUTTON_DOWN_GPIO_Port, 	BUTTON_DOWN_Pin, 	APPBOARD_BUTTON_DOWN},
		{BUTTON_LEFT_GPIO_Port, 	BUTTON_LEFT_Pin, 	APPBOARD_BUTTON_LEFT},
		{BUTTON_T1_GPIO_Port, 		BUTTON_T1_Pin, 		APPBOARD_BUTTON_T1},
		{BUTTON_T2_GPIO_Port, 		BUTTON_T2_Pin, 		APPBOARD_BUTTON_T2},
		{BUTTON_T3_GPIO_Port, 		BUTTON_T3_Pin, 		APPBOARD_BUTTON_T3}
};


uint32_t u32AppBoard_Buttons_ReadState(void)
{
	uint32_t returnMask = 0U;

	for(uint32_t i = 8; i > 0; i--)
	{

		if(eAppBoard_Buttons_IsButtonPressed(i) == Button_Pressed)
		{
			returnMask = returnMask | 1U;
		}
		returnMask = returnMask << 1;
	}

	return returnMask >> 1;
}

eButtonState_t eAppBoard_Buttons_IsButtonPressed(uint32_t u32Button)
{
	return --u32Button < 8 && !HAL_GPIO_ReadPin(casButtons[u32Button].psGPIO, casButtons[u32Button].u16Pin) ?  Button_Pressed : Button_NotPressed;
}
