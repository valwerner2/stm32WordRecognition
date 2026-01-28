//
// Created by valwe on 1/9/26.
//

#ifndef APPLICATION_H
#define APPLICATION_H

#pragma once

#include "main.h"
#include "gui.h"
#include "mfcc.h"

#define SAMPLE_RATE 16000

#define PREPROCESSING_BUFFER_SIZE  16000


extern TIM_HandleTypeDef htim2;
extern gui_info_t gui_info;
extern float mfcc_speech_window[MFCC_BUFFER_SIZE][NUM_DCT_OUTPUTS];

float samples[MFCC_FFT_LEN];
char is_ready_to_copy;
unsigned int samples_write_pos;
char flush_flag;

void Application_Init(void);
void Application_Start(void);
void Application_Process_Samples(void);
void Application_Timer2_Handler(void);
void Application_Error(const char* errorMsg);

#endif //APPLICATION_H
