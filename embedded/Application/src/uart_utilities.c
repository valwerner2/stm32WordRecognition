//
// Created by valwe on 1/28/26.
//

#include "uart_utilities.h"
#include <stdio.h>
#include "mfcc.h"
#include "AppBoard_LEDs.h"

int _write(int file, char *ptr, int len) {
    HAL_UART_Transmit(&huart1, (uint8_t *)ptr, len, HAL_MAX_DELAY);
    return len;
}

void UART_PrintFloatArray(int row, int col, float mfcc_data[row][col], char* mfcc_name, char python) {

    char bracketOpen = '{';
    char bracketClose = '}';
    if (python) {bracketOpen = '['; bracketClose = ']';}

    if(row > 1) {
        if(python) {
            printf("%s = %c\n", mfcc_name, bracketOpen);
        }else {
            printf("%s[%d][%d] = %c\n", mfcc_name, row, col, bracketOpen);
        }
    } else {
        if(python) {
            printf("%s = %c\n", mfcc_name, bracketOpen);
        }else {
            printf("%s[%d] = %c\n", mfcc_name, col, bracketOpen);
        }
    }



    for (int i = 0; i < row; i++) {
        if(row > 1){printf("  %c", bracketOpen);}
        for (int j = 0; j < col; j++) {
            if(python) {printf("%.6f", mfcc_data[i][j]);} else {printf("%.6ff", mfcc_data[i][j]);}
            if (j < col - 1) {
                printf(", ");
            }
        }
        if(row > 1){printf("%c", bracketClose);}

        if (i < row - 1) {
            printf(",");
        }
        printf("\n");
    }
    printf("%c;\n", bracketClose);
}
void UART_Flush_Detected_Speech(void)
{
    vAppBoard_LEDs_LEDOn(2);
    snprintf(gui_info.word, GUI_STR_LEN, "FLUSHED");
    gui_info.refresh = 1;
    UART_PrintFloatArray(mfcc_speech_window_size, NUM_DCT_OUTPUTS, mfcc_speech_window, "const float mfcc_", 0);
    vAppBoard_LEDs_LEDOff(2);
}