//
// Created by valwe on 1/28/26.
//

#include "mfcc.h"
#include <stdio.h>
#include "mfcc_lookup_tables.h"
#include "mfcc_sample_lookup_tables.h"
#include "arm_math.h"
#include "arm_const_structs.h"
#include "application.h"
#include "dtw.h"
#include "mfcc.h"
#include "AppBoard_LEDs.h"

const float dist_threshold = 0.035f;

void MFCC_Init(void) {
    mfcc_write_pos = 0;
    arm_status status = ARM_MATH_ARGUMENT_ERROR;
    if (MFCC_FFT_LEN == 1024) {
        status = arm_mfcc_init_1024_f32(
            &mfccInstance,
            NUM_MELS,
            NUM_DCT_OUTPUTS,
            mfcc_dct_coefs_dct_embedded_f32_1024,
            mfcc_filter_pos_mel_embedded_f32_1024,
            mfcc_filter_len_mel_embedded_f32_1024,
            mfcc_filter_coefs_mel_embedded_f32_1024,
            mfcc_window_coefs_window_embedded_f32_1024
        );
    }

    if (MFCC_FFT_LEN == 512) {
        status = arm_mfcc_init_512_f32(
            &mfccInstance,
            NUM_MELS,
            NUM_DCT_OUTPUTS,
            mfcc_dct_coefs_dct_embedded_f32_512,
            mfcc_filter_pos_mel_embedded_f32_512,
            mfcc_filter_len_mel_embedded_f32_512,
            mfcc_filter_coefs_mel_embedded_f32_512,
            mfcc_window_coefs_window_embedded_f32_512
        );
    }

    if (status != ARM_MATH_SUCCESS) {
        Application_Error("failed to initialize MFCC");
    }
}

char MFCC_Find_Speech_Interval(
    int mfcc_read_pos,
    const int speech_map_length,
    float mfcc_to_process[speech_map_length][NUM_DCT_OUTPUTS],
    const char shall_print,
    int *start_interval,
    int *end_interval)
{

    // copy
    char speech_map[speech_map_length];
    int speech_count = 0;
    for(int i = 0; i < speech_map_length; i++) {
        speech_map[i] = mfcc_to_process[mfcc_read_pos][0] >= noice_floor;
        speech_count += speech_map[i];
        mfcc_read_pos = (mfcc_read_pos+1) % speech_map_length;
    }

    if(speech_count < SPEECH_COUNT_THRESHOLD){return 0;}

    if(shall_print) {
        //unchanged buffer
        for (int i = 0; i < speech_map_length; i++) {
            printf("%d", (int)speech_map[i]);
        }
    }

    // fill gaps
    for(int i = 2; i < speech_map_length /2; i++) {
        if (speech_map[i-1] && speech_map[i - 2]) {
            speech_map[i] = 1;
        }
    }
    for(int i = speech_map_length - 1 - 2; i >= speech_map_length / 2; i--) {
        if (speech_map[i+1] && speech_map[i + 2]) {
            speech_map[i] = 1;
        }
    }

    if(shall_print) {
        printf(" - ");
        //changed buffer
        for (int i = 0; i < speech_map_length; i++) {
            printf("%d", (int)speech_map[i]);
        }
    }

    //find first valid match
    *start_interval = -1;
    for (int i = 1; *start_interval == -1 && i < speech_map_length; i++) {
        if (speech_map[i] && !speech_map[i - 1]) {
            *start_interval = i;
        }
    }
    *end_interval = -1;
    for(int i = *start_interval + 1; *end_interval == -1 && i < speech_map_length; i++) {
        if(!speech_map[i]) {
            *end_interval = i;
        }
    }

    int length = *end_interval - *start_interval;

    char ret_val = 0;
    if(*end_interval != -1 && *start_interval != -1 && length >= SPEECH_COUNT_THRESHOLD) {
        if(shall_print) {
            printf(" %d - %d | %d", *start_interval, *end_interval, length);
        }
        ret_val = 1;
    }

    if(shall_print) {
        printf("\n");
    }
    return ret_val;
}

void MFCC_Init_Noise_Floor(void) {
    for (int i = 0; i < MFCC_BUFFER_SIZE; i++) {
        noice_floor += mfcc_buffer[i][0];
    }
    const float mean = noice_floor / (float)MFCC_BUFFER_SIZE;

    float sum_root = 0.0f;
    for (size_t i = 0; i < MFCC_BUFFER_SIZE; i++) {
        const float diff = mfcc_buffer[i][0] - mean;
        sum_root += diff * diff;
    }

    const float std_div = sqrtf(sum_root / (float)(MFCC_BUFFER_SIZE- 1));

    noice_floor = mean + std_div * 4;
    printf("Noise floor = %f - (mean: %f, std_diff: %f)\n", noice_floor, mean, std_div);

    snprintf(gui_info.noise_floor, GUI_STR_LEN, "%f", noice_floor);
    gui_info.refresh = 1;
}

void MFCC_Process_Speech_Interval(const int start, const int count) {

    mfcc_speech_window_size = count;
    //empty speech!
    for(int i = 0; i < MFCC_BUFFER_SIZE; i++) {
        mfcc_buffer[i][0] = noice_floor - 2.f;
    }

    int start_actual = (mfcc_write_pos + start) % MFCC_BUFFER_SIZE; // speech map index 0 = mfccWritePos

    for(int i = 0; i < count; i++) {
        // first index is ignored (loudness)
        arm_copy_f32(&mfcc_buffer[start_actual][1], &mfcc_speech_window[i][1], NUM_DCT_OUTPUTS - 1);
        start_actual = (start_actual + 1) % MFCC_BUFFER_SIZE;
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
    static word_t wordLast = WORD_START;
    if(currentLowestDist < dist_threshold) {
        printf("Current lowest distance: %f - %d\n", currentLowestDist, mfcc_curr.word);
    }else {
        printf("Current lowest distance: %f - nothing found - %d\n", currentLowestDist, mfcc_curr.word);
        mfcc_curr.word = WORD_NOISE;
    }

    snprintf(gui_info.distance, GUI_STR_LEN, "%f", currentLowestDist);
    switch (mfcc_curr.word){
        case WORD_START:
            snprintf(gui_info.word, GUI_STR_LEN, "START");
            break;
        case WORD_STOP:
            snprintf(gui_info.word, GUI_STR_LEN, "STOP");
            break;
        case WORD_PLUS:
            snprintf(gui_info.word, GUI_STR_LEN, "PLUS");
            break;
        case WORD_MINUS:
            snprintf(gui_info.word, GUI_STR_LEN, "MINUS");
            break;
        default:
            snprintf(gui_info.word, GUI_STR_LEN, "?");
            break;
    }

    gui_info.refresh = 1;
    vAppBoard_LEDs_LEDOff(3 + wordLast);
    vAppBoard_LEDs_LEDOn(3 + mfcc_curr.word);
    wordLast = mfcc_curr.word;
}

void MFCC_Process_Frame(float *frame_mfcc_output, const float *frame_wave){
    float input_signal[MFCC_FFT_LEN];

    arm_copy_f32(frame_wave, input_signal, MFCC_FFT_LEN);
    arm_mfcc_f32(
        &mfccInstance,
        input_signal,
        frame_mfcc_output,
        mfcc_processing_buffer
    );
}