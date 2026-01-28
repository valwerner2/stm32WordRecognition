//
// Created by valwe on 1/28/26.
//

#ifndef UART_UTILITIES_H
#define UART_UTILITIES_H
#include "main.h"
#include "gui.h"

extern UART_HandleTypeDef huart1;
extern gui_info_t gui_info;

int _write(int file, char *ptr, int len);
void UART_PrintFloatArray(int row, int col, float mfccData[row][col], char* mfccName, char python);
void UART_Flush_Detected_Speech(void);

#endif //UART_UTILITIES_H
