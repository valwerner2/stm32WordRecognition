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

#include "mfcc_tabels.h"
#include <stdio.h>

#define MAX_POS_WAVE  16000

float samples[MFCC_FFT_LEN];

extern TIM_HandleTypeDef htim2;
extern UART_HandleTypeDef huart1;

arm_mfcc_instance_f32 mfccInstance;
float mfccTmp[MFCC_FFT_LEN + 2];
float noiseFloor = 0.0f;



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

void init_noise_floor(void) {
    for (int i = 0; i < mfccNoiseFlat.rows; i++) {
        noiseFloor += mfccNoiseFlat.data[i * NUM_DCT_OUTPUTS];
    }
    const float mean = noiseFloor / (float)mfccNoiseFlat.rows;

    float sumSQRT = 0.0f;
    for (size_t i = 0; i < mfccNoiseFlat.rows; i++) {
        float diff = mfccNoiseFlat.data[i * NUM_DCT_OUTPUTS] - mean;
        sumSQRT += diff * diff;
    }

    const float stdDiff = sqrtf(sumSQRT / (float)(mfccNoiseFlat.rows - 1));

    noiseFloor = mean + stdDiff * 3;
    printf("Noise floor = %f - (mean: %f, std_diff: %f)\n", noiseFloor, mean, stdDiff);
}

char samplesReady = 0;
unsigned int samplesWritePos = 0;

void samples_timer_handler(void) {
    static char firstFillUp = 1;
    static int amountReady = 0;

    // fill up buffer when first started
    if (firstFillUp) {
        if(samplesWritePos < MFCC_FFT_LEN) {
            samples[samplesWritePos] = f32AppBoard_ADC_ReadMicro();
            samplesWritePos++;

        }else{
            samplesWritePos = 0;
            samplesReady = 1;
            firstFillUp = 0;
        }
        return;
    }

    // exec mfcc code copied what it needed
    if (!samplesReady) {
        samples[samplesWritePos] = f32AppBoard_ADC_ReadMicro();
        samplesWritePos = (samplesWritePos + 1 ) % MFCC_FFT_LEN;
        amountReady++;

        if (amountReady == HOP_LEN) {
            samplesReady = 1;
            amountReady = 0;
        }
    }
}

void samples_processor_loop(void) {
    if(samplesReady) {
        float samples_mfcc[MFCC_FFT_LEN];

        // writer at start of array
        if(samplesWritePos == 0) {
            arm_copy_f32(samples, samples_mfcc, MFCC_FFT_LEN);
            samplesReady = 0;
            vAppBoard_LEDs_LEDOn(3);
            vAppBoard_LEDs_LEDOff(4);
            return;
        }

        // writer at middle of array
        if(samplesWritePos == HOP_LEN) {
            arm_copy_f32(samples, samples_mfcc + HOP_LEN - 1, HOP_LEN);
            arm_copy_f32(samples + HOP_LEN - 1, samples_mfcc, HOP_LEN);
            samplesReady = 0;
            vAppBoard_LEDs_LEDOn(4);
            vAppBoard_LEDs_LEDOff(3);
            return;
        }
        vAppBoard_LEDs_LEDOn(5);
        printf("samples_processor_loop: this shall not happen, pos: %d\n", samplesWritePos);
    }
}

void Application_Loop(void) {
    samples_processor_loop();
}

void Application_Init(void){
    mfcc_setup();
    init_noise_floor();
    HAL_TIM_Base_Start_IT(&htim2);

    while(1) {
        Application_Loop();
    }
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
    samples_timer_handler();

    static char makeWave = 0;

    if(eAppBoard_Buttons_IsButtonPressed(APPBOARD_BUTTON_CENTER)) {
        makeWave = 1;
    }

    if(makeWave) {Application_Make_Wave(&makeWave);}
    vAppBoard_LEDs_LEDOn(1);
}