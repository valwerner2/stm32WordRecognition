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
#define MFCC_BUFFER_SIZE 16

float samples[MFCC_FFT_LEN];

extern TIM_HandleTypeDef htim2;
extern UART_HandleTypeDef huart1;

arm_mfcc_instance_f32 mfccInstance;
float mfccTmp[MFCC_FFT_LEN + 2];
float noiseFloor = 0.0f;

const int speechCountThreshold = 5;

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
    vAppBoard_LEDs_LEDOn(2);
    arm_mfcc_f32(
        &mfccInstance,
        inputSignal,
        frameMfccOutput,
        mfccTmp
    );

    vAppBoard_LEDs_LEDOff(2);
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
        vAppBoard_LEDs_LEDOn(8);
    }
}

char samplesReady = 0;
unsigned int samplesWritePos = 0;

void samples_timer_handler(void) {
    static char firstFillUp = 1;
    static int amountReady = 0;
    static int skippedSamples = 0;

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
        //if(skippedSamples) {printf("skippedSamples: %d\n", skippedSamples); skippedSamples = 0;}
        samples[samplesWritePos] = f32AppBoard_ADC_ReadMicro();
        samplesWritePos = (samplesWritePos + 1) % MFCC_FFT_LEN;
        amountReady++;

        if (amountReady == HOP_LEN) {
            samplesReady = 1;
            amountReady = 0;
        }
    }else {
        skippedSamples++;
    }
}

float mfccBuffer[MFCC_BUFFER_SIZE][NUM_DCT_OUTPUTS];
int mfccWritePos = 0;

void init_noise_floor(void) {
    for (int i = 0; i < MFCC_BUFFER_SIZE; i++) {
        noiseFloor += mfccBuffer[i][0];
    }
    const float mean = noiseFloor / (float)MFCC_BUFFER_SIZE;

    float sumSQRT = 0.0f;
    for (size_t i = 0; i < MFCC_BUFFER_SIZE; i++) {
        const float diff = mfccBuffer[i][0] - mean;
        sumSQRT += diff * diff;
    }

    const float stdDiff = sqrtf(sumSQRT / (float)(MFCC_BUFFER_SIZE- 1));

    noiseFloor = mean + stdDiff * 4;
    printf("Noise floor = %f - (mean: %f, std_diff: %f)\n", noiseFloor, mean, stdDiff);
}
float distance_matrix[MFCC_BUFFER_SIZE][MFCC_BUFFER_SIZE];
float distance_buffer[MFCC_BUFFER_SIZE][MFCC_BUFFER_SIZE];

float compute_speech_dtw(const mfcc_t* mfcc1, const mfcc_t* mfcc2) {
    const int M = mfcc1->rows;
    const int N = mfcc2->rows;

    if(M > MFCC_BUFFER_SIZE || N > MFCC_BUFFER_SIZE){vAppBoard_LEDs_LEDOn(8); return MAXFLOAT;}
    arm_matrix_instance_f32 arm_dist_matrix = {M, N, (float32_t *)distance_matrix};
    arm_matrix_instance_f32 arm_dist_buffer = {M, N, (float32_t *)distance_buffer};

    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) {
            float dist = arm_euclidean_distance_f32(
                                        &mfcc1->data[i*NUM_DCT_OUTPUTS + 1], // ignore first
                                        &mfcc2->data[j*NUM_DCT_OUTPUTS + 1], // ignore first
                                        NUM_DCT_OUTPUTS - 1);
            distance_matrix[i][j] = dist;
        }
    }

    float result;
    arm_dtw_distance_f32(&arm_dist_matrix, 0, &arm_dist_buffer, &result);

    return result / (float)(M + N);
}

void process_speech_interval(const int start, const int count) {
    float mfcc_speech_window[count][NUM_DCT_OUTPUTS];

    int startActual = (mfccWritePos + start) % MFCC_BUFFER_SIZE; // speech map index 0 = mfccWritePos

    for(int i = 0; i < count; i++) {
        // first index is ignored (loudness)
        arm_copy_f32(&mfccBuffer[startActual][1], &mfcc_speech_window[i][1], NUM_DCT_OUTPUTS - 1);
        startActual = (startActual + 1) % MFCC_BUFFER_SIZE;
    }

    mfcc_t mfcc_curr = {
    .rows = count,
    .word = WORD_NOISE,
    .data = (float*)mfcc_speech_window};

    float currentLowestDist = MAXFLOAT;
    for(int i = 0;i < NUM_MFCC_COMPARISON; i++) {
        const float currentDist = compute_speech_dtw(&mfccComparison[i], &mfcc_curr);
        if(currentDist < currentLowestDist) {
            currentLowestDist = currentDist;
            mfcc_curr.word = mfccComparison[i].word;
        }
    }
    printf("Current lowest distance: %f - %d\n", currentLowestDist, mfcc_curr.word);
}

void mfcc_find_speech_interval(
    int mfccReadPos,
    const int speechMapLength,
    float mfccToProcess[speechMapLength][NUM_DCT_OUTPUTS],
    const char shallPrint,
    int *startInterval,
    int *endInterval) {

    // copy
    char speechMap[speechMapLength];
    for(int i = 0; i < speechMapLength; i++) {
        speechMap[i] = mfccToProcess[mfccReadPos][0] >= noiseFloor;
        mfccReadPos = (mfccReadPos+1) % speechMapLength;
    }

    if(shallPrint) {
        //unchanged buffer
        for (int i = 0; i < speechMapLength; i++) {
            printf("%d", (int)speechMap[i]);
        }
    }

    // fill gaps
    for(int i = 2; i < speechMapLength; i++) {
        if (speechMap[i] && speechMap[i - 2]) {
            speechMap[i-1] = 1;
        }
    }
    if(shallPrint) {
        printf(" - ");
        //changed buffer
        for (int i = 0; i < speechMapLength; i++) {
            printf("%d", (int)speechMap[i]);
        }
    }

    //find first valid match
    *startInterval = -1;
    for (int i = 1; *startInterval == -1 && i < speechMapLength; i++) {
        if (speechMap[i] && !speechMap[i - 1]) {
            *startInterval = i;
        }
    }
    *endInterval = -1;
    for(int i = *startInterval + 1; *endInterval == -1 && i < speechMapLength; i++) {
        if(!speechMap[i]) {
            *endInterval = i;
        }
    }

    int length = *endInterval - *startInterval;

    if(*endInterval != -1 && *startInterval != -1 && length >= speechCountThreshold) {
        if(shallPrint) {
            printf(" %d - %d | %d", *startInterval, *endInterval, length);
        }
        //empty speech!
        for(int i = 0; i < speechMapLength; i++) {
            mfccToProcess[i][0] = noiseFloor - 1.f;
        }
        vAppBoard_LEDs_LEDOn(3);
        vAppBoard_LEDs_LEDOff(3);
    }

    if(shallPrint) {
        printf("\n");
    }
}

void sample_processor_loop(void) {
    static char firstFillUp = 1;

    if(samplesReady) {
        float samples_mfcc[MFCC_FFT_LEN];
        char copied = 0;
        // writer at start of array
        if(samplesWritePos == 0) {
            arm_copy_f32(samples, samples_mfcc, MFCC_FFT_LEN);
            samplesReady = 0;
            vAppBoard_LEDs_LEDOn(1);
            copied = 1;
        }

        // writer at middle of array
        else if(samplesWritePos == HOP_LEN) {
            arm_copy_f32(samples, samples_mfcc + HOP_LEN - 1, HOP_LEN);
            arm_copy_f32(samples + HOP_LEN - 1, samples_mfcc, HOP_LEN);
            samplesReady = 0;
            vAppBoard_LEDs_LEDOff(1);
            copied = 1;
        }

        if (!copied) {
            vAppBoard_LEDs_LEDOn(8);
            printf("samples_processor_loop: this shall not happen, pos: %d\n", samplesWritePos);
            return;
        }

        //process copied
        process_frame(mfccBuffer[mfccWritePos], samples_mfcc);
        mfccWritePos = (mfccWritePos + 1) % MFCC_BUFFER_SIZE;


        if(!firstFillUp) {
            int startInterval = 0, endInterval = 0;
            mfcc_find_speech_interval(mfccWritePos,
                MFCC_BUFFER_SIZE,
                mfccBuffer,
                0,
                &startInterval,
                &endInterval);
            process_speech_interval(startInterval, endInterval - startInterval);
        } else if(mfccWritePos == 0 && firstFillUp) {
            init_noise_floor();
            firstFillUp = 0;
        }
    }
}



void Application_Loop(void) {
    sample_processor_loop();
}

void Application_Init(void){
    mfcc_setup();
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
}