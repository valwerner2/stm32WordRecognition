//
// Created by valwe on 1/28/26.
//

#include "dtw.h"
#include "main.h"
#include "application.h"

float compute_speech_dtw(const mfcc_t* mfcc1, const mfcc_t* mfcc2) {
    const int M = mfcc1->rows;
    const int N = mfcc2->rows;

    if(M > MFCC_BUFFER_SIZE || N > MFCC_BUFFER_SIZE) {
        Application_Error("MFCC buffer size is wrong");
    }
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