/*
 * encoder.c
 *
 *  Created on: Nov 16, 2025
 *      Author: yaofa
 */
#include "encoder.h"
#include "mpu6050.h"
#include "hardware_cfg.h"
#include "tim.h"
#include "stdio.h"

void Enc_Init() {
	// 左电机编码器(TIM3)
	enc_left.htim = &htim3;
	HAL_TIM_Base_Start_IT(enc_left.htim);
	HAL_TIM_Encoder_Start_IT(enc_left.htim, TIM_CHANNEL_ALL);

	// 右电机编码器(TIM5)
	enc_right.htim = &htim5;
	HAL_TIM_Base_Start_IT(enc_right.htim);
	HAL_TIM_Encoder_Start_IT(enc_right.htim, TIM_CHANNEL_ALL);

	// 开启基础定时器6
	HAL_TIM_Base_Start_IT(&htim6);
}


void LEFT_ENC_IRQHandler(void) {
	if(__HAL_TIM_GET_COUNTER(&htim3) < 32768)
		enc_left.total_overflow += 65536;
	else
		enc_left.total_overflow -= 65536;
}

void RIGHT_ENC_IRQHandler(void) {
	if(__HAL_TIM_GET_COUNTER(enc_right.htim) < 32768) {
		enc_right.total_overflow += 65536;
	} else {
		enc_right.total_overflow -= 65536;
	}
}

int32_t Calculate_RPM(int32_t pulse_diff) {
    // 公式：RPM = (pulse_diff * 60 * 1000) / (PPR * GearRatio * SamplePeriod_ms)
    return (pulse_diff * 60000.0f) /
          (ENCODER_RESOLUTION * GEAR_RATIO * SAMPLE_PERIOD_MS);
}

void ENC_UpdateSpeed(void) {
	// 左电机计算
	int32_t cnt_left = __HAL_TIM_GET_COUNTER(enc_left.htim);
	enc_left.diff = cnt_left + enc_left.total_overflow - enc_left.last_count;
	__HAL_TIM_DISABLE(enc_left.htim);
	__HAL_TIM_SET_COUNTER(enc_left.htim, 32768);
	__HAL_TIM_ENABLE(enc_left.htim);
	enc_left.last_count = 32768;
	enc_left.total_overflow = 0;

	// 右电机计算
	int32_t cnt_right = __HAL_TIM_GET_COUNTER(enc_right.htim);
	enc_right.diff = cnt_right  + enc_right.total_overflow - enc_right.last_count;
	__HAL_TIM_DISABLE(enc_right.htim);
	__HAL_TIM_SET_COUNTER(enc_right.htim, 32768);
	__HAL_TIM_ENABLE(enc_right.htim);
	enc_right.last_count = 32768;
	enc_right.total_overflow = 0;
}

void ENC_GetSpeeds(int32_t *left_rpm, int32_t *right_rpm) {
	__disable_irq();
	*left_rpm = enc_left.diff;
	*right_rpm = enc_right.diff;
	__enable_irq();
}


void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim) {
	__disable_irq();
	if (htim == enc_left.htim) {
	} else if (htim == enc_right.htim) {
	}
	__enable_irq();
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
	__disable_irq();
	if (htim->Instance == TIM3) {
		LEFT_ENC_IRQHandler();
	} else if (htim->Instance == TIM5) {
		RIGHT_ENC_IRQHandler();
	} else if (htim->Instance == TIM6) {
		ENC_UpdateSpeed();
//		MPU_Update();
	}
	__enable_irq();
}

void MPU_Update() {
	float roll, yaw, pitch;
	MPU6050_DMP_Get_Date(&pitch, &roll, &yaw);
	printf("%.2f, %.2f, %.2f.\r\n", pitch, roll, yaw);
}
