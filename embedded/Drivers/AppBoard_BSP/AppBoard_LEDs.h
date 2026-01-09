//
// Created by valwe on 10/9/25.
//

#ifndef ES2V1_APPBOARDLEDS_H
#define ES2V1_APPBOARDLEDS_H

#include "main.h"

#define APPBOARD_NUMBER_LEDS 8U
#define APPBOARD_LED1 (1U << 0U)
#define APPBOARD_LED2 (1U << 1U)
#define APPBOARD_LED3 (1U << 2U)
#define APPBOARD_LED4 (1U << 3U)
#define APPBOARD_LED5 (1U << 4U)
#define APPBOARD_LED6 (1U << 5U)
#define APPBOARD_LED7 (1U << 6U)
#define APPBOARD_LED8 (1U << 7U)
#define APPBOARD_LEDS_ALL_OFF (0x00)
#define APPBOARD_LEDS_ALL_ON (0xFF)

typedef struct
{
    GPIO_TypeDef* psGPIO;
    uint16_t u16Pin;
}sPinIDLEDs_t;

void vAppBoard_LEDs_Write(uint32_t u32Val);
void vAppBoard_LEDs_LEDOn(uint32_t u32LED);
void vAppBoard_LEDs_LEDOff(uint32_t u32LED);

#endif //ES2V1_APPBOARDLEDS_H
