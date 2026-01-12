//
// Created by valwe on 1/12/26.
//
#include <stdio.h>

#include "gui.h"
#include "stm32f429i_discovery_lcd.h"
#include "fonts.h"

extern gui_info_t gui_info;

void gui_init(){

    gui_info.refresh = 0;
    snprintf(gui_info.noise_floor, GUI_STR_LEN, "...");
    snprintf(gui_info.distance, GUI_STR_LEN, "...");
    snprintf(gui_info.word, GUI_STR_LEN, "-");

    BSP_LCD_Init();
    BSP_LCD_LayerDefaultInit(LCD_BACKGROUND_LAYER,LCD_FRAME_BUFFER);
    BSP_LCD_LayerDefaultInit(LCD_FOREGROUND_LAYER,LCD_FRAME_BUFFER);
    BSP_LCD_SelectLayer(LCD_FOREGROUND_LAYER);
    BSP_LCD_DisplayOn();
    gui_update();
    gui_info.refresh = 0;
}

void gui_update(){
    if(!gui_info.refresh){return;}

    gui_info.refresh = 0;
    BSP_LCD_Clear(LCD_COLOR_BLUE);
    BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
    BSP_LCD_SetBackColor(LCD_COLOR_WHITE);
    BSP_LCD_FillRect(10, 10, 220, 300);
    BSP_LCD_SetTextColor(LCD_COLOR_BLACK);

    BSP_LCD_SetFont(&Font16);
    BSP_LCD_DisplayStringAt(10, 20, (uint8_t*)"stm32WordRecognition", LEFT_MODE);
    BSP_LCD_SetFont(&Font12);
    BSP_LCD_DisplayStringAt(10, 45, (uint8_t*)"github.com/valwerner2", LEFT_MODE);
    BSP_LCD_SetFont(&Font16);
    BSP_LCD_DrawHLine(10, 70, 220);

    BSP_LCD_DisplayStringAt(20, 100, (uint8_t*)"NoiseFloor:", LEFT_MODE);
    BSP_LCD_DisplayStringAt(20, 120, (uint8_t*)gui_info.noise_floor, LEFT_MODE);

    BSP_LCD_DisplayStringAt(20, 160, (uint8_t*)"Distance:", LEFT_MODE);
    BSP_LCD_DisplayStringAt(20, 180, (uint8_t*)gui_info.distance, LEFT_MODE);

    BSP_LCD_DrawHLine(10, 225, 220);
    BSP_LCD_DisplayStringAt(20, 230, (uint8_t*)"Recognized:", LEFT_MODE);
    BSP_LCD_SetFont(&Font24);
    BSP_LCD_DisplayStringAt(0, 262, (uint8_t*)gui_info.word, CENTER_MODE);
}