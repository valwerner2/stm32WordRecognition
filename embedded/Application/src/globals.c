/*
 * globals.c
 *
 *  Created on: Oct 23, 2025
 *      Author: valwe
 */

#include "gui.h"
#include "mfcc.h"

gui_info_t gui_info = {{0}};
float mfcc_buffer[MFCC_BUFFER_SIZE][NUM_DCT_OUTPUTS];
float mfcc_speech_window[MFCC_BUFFER_SIZE][NUM_DCT_OUTPUTS];


