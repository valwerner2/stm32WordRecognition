//
// Created by valwe on 1/28/26.
//

#ifndef MFCC_H
#define MFCC_H

#include "arm_math.h"
#include "arm_const_structs.h"
#include "gui.h"


#define SPEECH_COUNT_THRESHOLD 15
#define NUM_DCT_OUTPUTS 13
#define MFCC_BUFFER_SIZE 40
#define MFCC_FFT_LEN 512
#define NUM_MELS 40
#define HOP_LEN 256

typedef enum word{
    WORD_START = 0,
    WORD_STOP,
    WORD_PLUS,
    WORD_MINUS,
    WORD_NOISE
}word_t;

typedef struct{
    int rows;
    word_t word;
    const float *data;
}mfcc_t;

extern gui_info_t gui_info;
extern float mfcc_buffer[MFCC_BUFFER_SIZE][NUM_DCT_OUTPUTS];
extern float mfcc_speech_window[MFCC_BUFFER_SIZE][NUM_DCT_OUTPUTS];

float mfcc_processing_buffer[MFCC_FFT_LEN * 2];
float distance_matrix[MFCC_BUFFER_SIZE][MFCC_BUFFER_SIZE];
float distance_buffer[MFCC_BUFFER_SIZE][MFCC_BUFFER_SIZE];

arm_mfcc_instance_f32 mfccInstance;
int mfcc_speech_window_size;
int mfcc_write_pos;
float noice_floor;

void MFCC_Init(void);
char MFCC_Find_Speech_Interval(
    int mfcc_read_pos,
    const int speech_map_length,
    float mfcc_to_process[speech_map_length][NUM_DCT_OUTPUTS],
    const char shall_print,
    int *start_interval,
    int *end_interval
);
void MFCC_Init_Noise_Floor(void);
void MFCC_Process_Speech_Interval(const int start, const int count);
void MFCC_Process_Frame(float *frame_mfcc_output, const float *frame_wave);

#endif //MFCC_H
