//
// Created by valwe on 1/9/26.
//

#include "application.h"
#include "AppBoard_LEDs.h"
#include "AppBoard_Buttons.h"
#include "AppBoard_ADC.h"

#include "uart_utilities.h"

#include <stdio.h>

void Application_Timer2_Collect_Sample(void) {
    static char is_first_fill_up = 1;
    static int amount_ready = 0;
    static int skipped_samples = 0;

    // fill up buffer when first started
    if (is_first_fill_up) {
        if(samples_write_pos < MFCC_FFT_LEN) {
            samples[samples_write_pos] = f32AppBoard_ADC_ReadMicro();
            samples_write_pos++;
        }else{
            samples_write_pos = 0;
            is_ready_to_copy = 1;
            is_first_fill_up = 0;
        }
        return;
    }

    // exec mfcc code copied what it needed
    if (!is_ready_to_copy) {
        //if(skippedSamples) {printf("skipped_samples: %d\n", skipped_samples); skipped_samples = 0;}
        samples[samples_write_pos] = f32AppBoard_ADC_ReadMicro();
        samples_write_pos = (samples_write_pos + 1) % MFCC_FFT_LEN;
        amount_ready++;

        if (amount_ready == HOP_LEN) {
            is_ready_to_copy = 1;
            amount_ready = 0;
        }
    }else {
        skipped_samples++;
    }
}

void Application_Process_Samples(void) {
    static char is_first_fill_up = 1;

    if(is_ready_to_copy) {
        float samples_mfcc[MFCC_FFT_LEN];
        // writer at start of array
        if(samples_write_pos == 0) {
            arm_copy_f32(samples, samples_mfcc, MFCC_FFT_LEN);
            is_ready_to_copy = 0;
            vAppBoard_LEDs_LEDOn(1);
        }

        // writer at middle of array
        else if(samples_write_pos == HOP_LEN) {
            arm_copy_f32(samples, samples_mfcc + HOP_LEN - 1, HOP_LEN);
            arm_copy_f32(samples + HOP_LEN - 1, samples_mfcc, HOP_LEN);
            is_ready_to_copy = 0;
            vAppBoard_LEDs_LEDOff(1);
        }
        else {
            vAppBoard_LEDs_LEDOn(8);
            printf("samples_processor_loop: this shall not happen, pos: %d\n", samples_write_pos);
        }

        //process copied
        MFCC_Process_Frame(mfcc_buffer[mfcc_write_pos], samples_mfcc);
        mfcc_write_pos = (mfcc_write_pos + 1) % MFCC_BUFFER_SIZE;


        if(!is_first_fill_up) {
            int startInterval = 0, endInterval = 0;
            if (MFCC_Find_Speech_Interval(mfcc_write_pos,
                MFCC_BUFFER_SIZE,
                mfcc_buffer,
                0,
                &startInterval,
                &endInterval)) {

                vAppBoard_LEDs_LEDOn(2);
                MFCC_Process_Speech_Interval(startInterval, endInterval - startInterval);
                vAppBoard_LEDs_LEDOff(2);
            }

        } else if(mfcc_write_pos == 0 && is_first_fill_up) {
            MFCC_Init_Noise_Floor();
            is_first_fill_up = 0;
        }
    }
}

void Application_Init(void){
    is_ready_to_copy = 0;
    samples_write_pos = 0;
    flush_flag = 0;

    HAL_TIM_Base_Start_IT(&htim2);
    MFCC_Init();
    Gui_Init();
}

void Application_Start(void){
    for(;;) {
        Application_Process_Samples();
        Gui_Update();
        if (flush_flag) {
            UART_Flush_Detected_Speech();
            flush_flag = 0;
        }
    }
}

void Application_Timer2_Handler(void){
    if (!flush_flag){
        Application_Timer2_Collect_Sample();

        if (eAppBoard_Buttons_IsButtonPressed(APPBOARD_BUTTON_CENTER)){
            flush_flag = 1;
        }
    }

}

void Application_Error(const char* errorMsg)
{
  printf("%s\n", errorMsg);
  vAppBoard_LEDs_LEDOn(8);

  for(;;){}
}