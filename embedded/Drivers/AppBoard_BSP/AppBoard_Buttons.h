#ifndef _APPBOARD_BUTTONS_H
#define _APPBOARD_BUTTONS_H

#include "main.h"

#define APPBOARD_NUMBER_BUTTONS 8U

typedef enum eButtonState {Button_NotPressed = 0, Button_Pressed = 1} eButtonState_t;

#define APPBOARD_BUTTON_CENTER (1U << 0U)
#define APPBOARD_BUTTON_UP (1U << 1U)
#define APPBOARD_BUTTON_RIGHT (1U << 2U)
#define APPBOARD_BUTTON_DOWN (1U << 3U)
#define APPBOARD_BUTTON_LEFT (1U << 4U)
#define APPBOARD_BUTTON_T1 (1U << 5U)
#define APPBOARD_BUTTON_T2 (1U << 6U)
#define APPBOARD_BUTTON_T3 (1U << 7U)

typedef struct
{
 GPIO_TypeDef* psGPIO;
 uint16_t u16Pin;
 uint32_t u32ButtonMask;
}sPinIDButtons_t;


uint32_t u32AppBoard_Buttons_ReadState(void);
eButtonState_t eAppBoard_Buttons_IsButtonPressed(uint32_t u32Button);
#endif //_APPBOARD_BUTTONS_H
