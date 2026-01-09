//
// Created by valwe on 1/9/26.
//

#ifndef APPLICATION_H
#define APPLICATION_H

#define SAMPLE_RATE 16000
#define MFCC_FFT_LEN 1024
#define NUM_DCT_OUTPUTS 13
#define NUM_MELS 40
#define HOP_LEN 512

void Application_Init(void);
void Application_Timer2_Handler(void);

#endif //APPLICATION_H
