//
// Created by valwe on 1/9/26.
//

#include "Application.h"
#include "main.h"
#include "AppBoard_LEDs.h"
#include "mfccdata.h"
#include "arm_math.h"
#include "arm_const_structs.h"
#include "startJ.h"
#include "stopJ.h"
#include "minusJ.h"
#include "plusJ.h"
#include <stdio.h>

extern TIM_HandleTypeDef htim2;
extern UART_HandleTypeDef huart1;

arm_mfcc_instance_f32 mfccInstance;
float mfccTmp[MFCC_FFT_LEN + 2];

int _write(int file, char *ptr, int len) {
    HAL_UART_Transmit(&huart1, (uint8_t *)ptr, len, HAL_MAX_DELAY);
    return len;
}

void PrintMFCCArray(int mfccLength, float mfccData[mfccLength][NUM_DCT_OUTPUTS]) {
    // Print the array declaration
    printf("const float mfcc_const_data[%d][%d] = [\n", mfccLength, NUM_DCT_OUTPUTS);

    for (int i = 0; i < mfccLength; i++) {
        printf("  [");
        for (int j = 0; j < NUM_DCT_OUTPUTS; j++) {
            printf("%.6f", mfccData[i][j]);
            if (j < NUM_DCT_OUTPUTS - 1) {
                printf(", ");
            }
        }
        printf("]");

        if (i < mfccLength - 1) {
            printf(",");
        }
        printf("\n");
    }
    printf("];\n");
}

int calc_spectrum_len(unsigned int lengthWav) {
    return floor(lengthWav / HOP_LEN) - 1;
}

void process_frame(float *frameMfccOutput, float *frameWave) {
    float inputSignal[MFCC_FFT_LEN];

    arm_copy_f32(frameWave, inputSignal, MFCC_FFT_LEN);

    arm_mfcc_f32(
        &mfccInstance,
        inputSignal,
        frameMfccOutput,
        mfccTmp
    );
}

void preprocess_wave(unsigned int length_wav, float *wave) {
    int mfcc_len = calc_spectrum_len(length_wav);
    float mfccOutput[mfcc_len][NUM_DCT_OUTPUTS];

    for(int currentFrame = 0; currentFrame < mfcc_len; currentFrame++) {
        process_frame(mfccOutput[currentFrame], wave + HOP_LEN * currentFrame);
    }

    PrintMFCCArray(mfcc_len, mfccOutput);

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
    HAL_TIM_Base_Start_IT(&htim2);
    mfcc_setup();
    vAppBoard_LEDs_LEDOn(2);
    preprocess_wave(startJ_len, startJ);
    vAppBoard_LEDs_LEDOn(3);
    preprocess_wave(stopJ_len, stopJ);
    vAppBoard_LEDs_LEDOn(4);
    preprocess_wave(minusJ_len, minusJ);
    vAppBoard_LEDs_LEDOn(5);
    preprocess_wave(plusJ_len, plusJ);
    vAppBoard_LEDs_LEDOn(6);
}

void Application_Timer2_Handler(void){
    vAppBoard_LEDs_LEDOn(1);
}