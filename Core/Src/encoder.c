/*
 * encoder.c
 *
 *  Created on: Nov 16, 2025
 *      Author: yaofa
 */
#include "encoder.h"
#include "hardware_cfg.h"
#include "tim.h"

#define ENC_CNT_MID   32768
#define ENC_CNT_FULL  65536

static Encoder enc_left;
static Encoder enc_right;

static void ENC_ResetCounter(Encoder *enc)
{
	__HAL_TIM_SET_COUNTER(enc->htim, ENC_CNT_MID);
	enc->last_count = ENC_CNT_MID;
	enc->total_overflow = 0;
}

static void ENC_OnOverflow(Encoder *enc)
{
	/* DIR=1: counting down → underflow; DIR=0: counting up → overflow */
	if (__HAL_TIM_IS_TIM_COUNTING_DOWN(enc->htim)) {
		enc->total_overflow -= ENC_CNT_FULL;
	} else {
		enc->total_overflow += ENC_CNT_FULL;
	}
}

static void ENC_SampleChannel(Encoder *enc)
{
	int32_t cnt = (int32_t)__HAL_TIM_GET_COUNTER(enc->htim);
	int32_t diff = cnt + enc->total_overflow - enc->last_count;

	/*
	 * If wrap happened while TIM6 callback holds IRQs disabled, UIF is set
	 * but the encoder overflow handler has not run yet. Fold it into this
	 * sample and clear the flag so the pending IRQ does not double-count.
	 */
	if (__HAL_TIM_GET_FLAG(enc->htim, TIM_FLAG_UPDATE) != RESET) {
		if (__HAL_TIM_IS_TIM_COUNTING_DOWN(enc->htim)) {
			diff -= ENC_CNT_FULL;
		} else {
			diff += ENC_CNT_FULL;
		}
		__HAL_TIM_CLEAR_FLAG(enc->htim, TIM_FLAG_UPDATE);
	}

	enc->diff = diff;
	enc->total += diff;
	/* Keep timer running; only recenter CNT (avoid missing A/B edges). */
	__HAL_TIM_SET_COUNTER(enc->htim, ENC_CNT_MID);
	enc->last_count = ENC_CNT_MID;
	enc->total_overflow = 0;
}

void Enc_Init(void)
{
	enc_left.htim = &htim3;
	enc_right.htim = &htim5;
	enc_left.total = 0;
	enc_right.total = 0;

	HAL_TIM_Base_Start_IT(enc_left.htim);
	HAL_TIM_Encoder_Start_IT(enc_left.htim, TIM_CHANNEL_ALL);

	HAL_TIM_Base_Start_IT(enc_right.htim);
	HAL_TIM_Encoder_Start_IT(enc_right.htim, TIM_CHANNEL_ALL);

	ENC_ResetCounter(&enc_left);
	ENC_ResetCounter(&enc_right);
	__HAL_TIM_CLEAR_FLAG(enc_left.htim, TIM_FLAG_UPDATE);
	__HAL_TIM_CLEAR_FLAG(enc_right.htim, TIM_FLAG_UPDATE);

	HAL_TIM_Base_Start_IT(&htim6);
}

float Calculate_RPM(int32_t pulse_diff)
{
	/* RPM = (pulse_diff * 60 * 1000) / (PPR * GearRatio * SamplePeriod_ms) */
	return (pulse_diff * 60000.0f) /
	       (float)(ENCODER_RESOLUTION * GEAR_RATIO * SAMPLE_PERIOD_MS);
}

void ENC_UpdateSpeed(void)
{
	ENC_SampleChannel(&enc_left);
	ENC_SampleChannel(&enc_right);
}

void ENC_GetSpeeds(float *left_rpm, float *right_rpm)
{
	int32_t left_diff;
	int32_t right_diff;

	__disable_irq();
	left_diff = enc_left.diff;
	right_diff = enc_right.diff;
	__enable_irq();

	*left_rpm = Calculate_RPM(left_diff);
	*right_rpm = Calculate_RPM(right_diff);
}

void ENC_GetPulseDiffs(int32_t *left_diff, int32_t *right_diff)
{
	__disable_irq();
	*left_diff = enc_left.diff;
	*right_diff = enc_right.diff;
	__enable_irq();
}

void ENC_GetTotals(int32_t *left_total, int32_t *right_total)
{
	__disable_irq();
	*left_total = enc_left.total;
	*right_total = enc_right.total;
	__enable_irq();
}

void ENC_GetRawCnt(uint32_t *left_cnt, uint32_t *right_cnt)
{
	*left_cnt = __HAL_TIM_GET_COUNTER(enc_left.htim);
	*right_cnt = __HAL_TIM_GET_COUNTER(enc_right.htim);
}

void ENC_ResetTotals(void)
{
	__disable_irq();
	enc_left.total = 0;
	enc_right.total = 0;
	__enable_irq();
}

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
	(void)htim;
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	__disable_irq();
	if (htim->Instance == TIM3) {
		ENC_OnOverflow(&enc_left);
	} else if (htim->Instance == TIM5) {
		ENC_OnOverflow(&enc_right);
	} else if (htim->Instance == TIM6) {
		ENC_UpdateSpeed();
	}
	__enable_irq();
}
