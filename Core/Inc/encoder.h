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
} Encoder;

static Encoder enc_left, enc_right;

void Enc_Init();
void LEFT_ENC_IRQHandler(void);
void RIGHT_ENC_IRQHandler(void);
int32_t Calculate_RPM(int32_t pulse_diff);
void ENC_UpdateSpeed(void);
void ENC_GetSpeeds(int32_t *left_rpm, int32_t *right_rpm);
void MPU_Update();

#endif /* INC_ENCODER_H_ */
