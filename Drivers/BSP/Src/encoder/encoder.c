/*
 * encoder.c
 *
 *  Created on: Jan 18, 2026
 *      Author: yaofa
 */
#include "encoder.h"

volatile int32_t encoderA_count = 0;
volatile int32_t encoderB_count = 0;
volatile enum MEASURED_TYPE encoderA_type = M;
volatile enum MEASURED_TYPE encoderB_type = M;

void Enc_init() {
    // 启动TIM5和TIM6
    HAL_TIM_Encoder_Start(&htim5, TIM_CHANNEL_ALL);
    HAL_TIM_Base_Start_IT(&htim6);

    // 启动TIM3和TIM7
    HAL_TIM_Encoder_Start(&htim3, TIM_CHANNEL_ALL);
    HAL_TIM_Base_Start_IT(&htim7);
}
