//
// Created by valwe on 1/9/26.
//

#include "Application.h"
#include "main.h"
#include "AppBoard_LEDs.h"
#include "AppBoard_Buttons.h"
#include "AppBoard_ADC.h"
#include "mfccdata.h"
#include "arm_math.h"
#include "arm_const_structs.h"
#include "start0.h"
#include "start1.h"
#include "start2.h"

#include "stop0.h"
#include "stop1.h"
#include "stop2.h"

#include "minus0.h"
#include "minus1.h"
#include "minus2.h"

#include "plus0.h"
#include "plus1.h"
#include "plus2.h"

#include "noice.h"

#include <stdio.h>

extern TIM_HandleTypeDef htim2;
extern UART_HandleTypeDef huart1;

arm_mfcc_instance_f32 mfccInstance;
float mfccTmp[MFCC_FFT_LEN + 2];
#define MAX_POS_WAVE  16000

int _write(int file, char *ptr, int len) {
    HAL_UART_Transmit(&huart1, (uint8_t *)ptr, len, HAL_MAX_DELAY);
    return len;
}

void PrintIntArray(int row, int col, int16_t mfccData[row][col], char* mfccName, char python) {

    char bracketOpen = '{';
    char bracketClose = '}';
    if (python) {bracketOpen = '['; bracketClose = ']';}

    if(row > 1) {
        if(python) {
            printf("%s = %c\n", mfccName, bracketOpen);
        }else {
            printf("%s[%d][%d] = %c\n", mfccName, row, col, bracketOpen);
        }
    } else {
        if(python) {
            printf("%s = %c\n", mfccName, bracketOpen);
        }else {
            printf("%s[%d] = %c\n", mfccName, col, bracketOpen);
        }
    }



    for (int i = 0; i < row; i++) {
        if(row > 1){printf("  %c", bracketOpen);}
        for (int j = 0; j < col; j++) {
            printf("%d", mfccData[i][j]);
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

void PrintFloatArray(int row, int col, float mfccData[row][col], char* mfccName, char python) {

    char bracketOpen = '{';
    char bracketClose = '}';
    if (python) {bracketOpen = '['; bracketClose = ']';}

    if(row > 1) {
        if(python) {
            printf("%s = %c\n", mfccName, bracketOpen);
        }else {
            printf("%s[%d][%d] = %c\n", mfccName, row, col, bracketOpen);
        }
    } else {
        if(python) {
            printf("%s = %c\n", mfccName, bracketOpen);
        }else {
            printf("%s[%d] = %c\n", mfccName, col, bracketOpen);
        }
    }



    for (int i = 0; i < row; i++) {
        if(row > 1){printf("  %c", bracketOpen);}
        for (int j = 0; j < col; j++) {
            if(python) {printf("%.6f", mfccData[i][j]);} else {printf("%.6ff", mfccData[i][j]);}
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

int calc_spectrum_len(unsigned int lengthWav) {
    return floor(lengthWav / HOP_LEN) - 1;
}

void process_frame(float *frameMfccOutput, const float *frameWave) {
    float inputSignal[MFCC_FFT_LEN];

    arm_copy_f32(frameWave, inputSignal, MFCC_FFT_LEN);

    arm_mfcc_f32(
        &mfccInstance,
        inputSignal,
        frameMfccOutput,
        mfccTmp
    );
}

void preprocess_wave(unsigned int length_wav, const float *wave, char* waveName) {
    int mfcc_len = calc_spectrum_len(length_wav);
    float mfccOutput[mfcc_len][NUM_DCT_OUTPUTS];

    for(int currentFrame = 0; currentFrame < mfcc_len; currentFrame++) {
        process_frame(mfccOutput[currentFrame], wave + HOP_LEN * currentFrame);
    }

    PrintFloatArray(mfcc_len, NUM_DCT_OUTPUTS, mfccOutput, waveName, 0);

}

void mfcc_setup(void) {
    arm_status status = arm_mfcc_init_1024_f32(
        &mfccInstance,
        NUM_MELS,
        NUM_DCT_OUTPUTS,
        mfcc_dct_coefs_dct_embedded_f32,
        mfcc_filter_pos_mel_embedded_f32,
        mfcc_filter_len_mel_embedded_f32,
        mfcc_filter_coefs_mel_embedded_f32,
        mfcc_window_coefs_window_embedded_f32
    );

    if (status != ARM_MATH_SUCCESS) {
        vAppBoard_LEDs_LEDOn(7);
    }
}

void Application_Init(void){
    mfcc_setup();
    preprocess_wave(start0_len, start0, "const float mfcc_start0");
    preprocess_wave(start1_len, start1, "const float mfcc_start1");
    preprocess_wave(start2_len, start2, "const float mfcc_start2");

    preprocess_wave(stop0_len, stop0, "const float mfcc_stop0");
    preprocess_wave(stop1_len, stop1, "const float mfcc_stop1");
    preprocess_wave(stop2_len, stop2, "const float mfcc_stop2");

    preprocess_wave(minus0_len, minus0, "const float mfcc_minus0");
    preprocess_wave(minus1_len, minus1, "const float mfcc_minus1");
    preprocess_wave(minus2_len, minus2, "const float mfcc_minus2");

    preprocess_wave(plus0_len, plus0, "const float mfcc_plus0");
    preprocess_wave(plus1_len, plus1, "const float mfcc_plus1");
    preprocess_wave(plus2_len, plus2, "const float mfcc_plus2");

    preprocess_wave(noice_len, noice, "const float mfcc_noice");
    HAL_TIM_Base_Start_IT(&htim2);
}

void Application_Make_Wave(char* makeWave) {
    static int currPos = 0;
    vAppBoard_LEDs_LEDOn(8);

    static int16_t wave[1][MAX_POS_WAVE] = {0};

    if(currPos < MAX_POS_WAVE) {
        wave[0][currPos] = s16AppBoard_ADC_ReadMicro();
        currPos++;
    }else {
        currPos = 0;
        *makeWave = 0;
        vAppBoard_LEDs_LEDOff(8);
        PrintIntArray(1, MAX_POS_WAVE, wave, "newWave", 1);
    }

}

void Application_Timer2_Handler(void){
    static char makeWave = 0;

    if(eAppBoard_Buttons_IsButtonPressed(APPBOARD_BUTTON_CENTER)) {
        makeWave = 1;
    }

    if(makeWave) {Application_Make_Wave(&makeWave);}
    vAppBoard_LEDs_LEDOn(1);
}