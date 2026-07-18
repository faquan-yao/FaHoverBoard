/*
 * encoder.h
 *
 *  Created on: Nov 16, 2025
 *      Author: yaofa
 */

#ifndef INC_ENCODER_H_
#define INC_ENCODER_H_

#include "tim.h"
#include "stdint.h"

typedef struct {
    TIM_HandleTypeDef *htim;
    int32_t last_count;
    int32_t total_overflow;
    int32_t diff;
    int32_t total; /* cumulative pulses since Enc_Init / ENC_ResetTotals */
} Encoder;

void Enc_Init(void);
float Calculate_RPM(int32_t pulse_diff);
void ENC_UpdateSpeed(void);
void ENC_GetSpeeds(float *left_rpm, float *right_rpm);
void ENC_GetPulseDiffs(int32_t *left_diff, int32_t *right_diff);
void ENC_GetTotals(int32_t *left_total, int32_t *right_total);
void ENC_GetRawCnt(uint32_t *left_cnt, uint32_t *right_cnt);
void ENC_ResetTotals(void);

#endif /* INC_ENCODER_H_ */
