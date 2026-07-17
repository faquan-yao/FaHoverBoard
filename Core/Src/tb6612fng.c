#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "tb6612fng.h"
#include "gpio.h"
#include "main.h"
#include "tim.h"

static void PwmPins_AsGpioHigh(void)
{
	GPIO_InitTypeDef gpio = {0};

	HAL_TIM_PWM_Stop(&htim4, TIM_CHANNEL_1);
	HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);

	/* Drive PWMA/PWMB as solid HIGH (equivalent to 100% duty) */
	gpio.Mode = GPIO_MODE_OUTPUT_PP;
	gpio.Pull = GPIO_NOPULL;
	gpio.Speed = GPIO_SPEED_FREQ_HIGH;

	gpio.Pin = PWMA_Pin;
	HAL_GPIO_Init(PWMA_GPIO_Port, &gpio);
	HAL_GPIO_WritePin(PWMA_GPIO_Port, PWMA_Pin, GPIO_PIN_SET);

	gpio.Pin = PWMB_Pin;
	HAL_GPIO_Init(PWMB_GPIO_Port, &gpio);
	HAL_GPIO_WritePin(PWMB_GPIO_Port, PWMB_Pin, GPIO_PIN_SET);
}

static void PwmPins_RestoreAf(void)
{
	/* Re-run Cube post-init path via PWM start after MspPostInit pins */
	HAL_TIM_MspPostInit(&htim4);
	HAL_TIM_MspPostInit(&htim1);
	HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_1);
	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
	__HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, 0);
	__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 0);
}

void MotorInit(void)
{
	HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_1);  /* PWMA = PD12 / TIM4_CH1 */
	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);  /* PWMB = PA8  / TIM1_CH1 */

	HAL_GPIO_WritePin(STBY_GPIO_Port, STBY_Pin, GPIO_PIN_SET);

	MotorAControl(MOTOR_STOP, 0);
	MotorBControl(MOTOR_STOP, 0);
}

/*
 * Force motors on without relying on TIM PWM.
 * If wheels still do not move: check VM motor power, wiring, STBY, AO/BO.
 */
void MotorHwTest(void)
{
	printf("Motor DC test: STBY=1, IN CW, PWM pins GPIO HIGH (2s)\r\n");

	PwmPins_AsGpioHigh();

	HAL_GPIO_WritePin(STBY_GPIO_Port, STBY_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(AIN1_GPIO_Port, AIN1_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(AIN2_GPIO_Port, AIN2_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(BIN1_GPIO_Port, BIN1_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(BIN2_GPIO_Port, BIN2_Pin, GPIO_PIN_RESET);

	printf("  STBY=%d AIN1=%d AIN2=%d BIN1=%d BIN2=%d PWMA=%d PWMB=%d\r\n",
		   (int)HAL_GPIO_ReadPin(STBY_GPIO_Port, STBY_Pin),
		   (int)HAL_GPIO_ReadPin(AIN1_GPIO_Port, AIN1_Pin),
		   (int)HAL_GPIO_ReadPin(AIN2_GPIO_Port, AIN2_Pin),
		   (int)HAL_GPIO_ReadPin(BIN1_GPIO_Port, BIN1_Pin),
		   (int)HAL_GPIO_ReadPin(BIN2_GPIO_Port, BIN2_Pin),
		   (int)HAL_GPIO_ReadPin(PWMA_GPIO_Port, PWMA_Pin),
		   (int)HAL_GPIO_ReadPin(PWMB_GPIO_Port, PWMB_Pin));
	printf("  Measure NOW on MCU pins / TB6612 inputs for 10s...\r\n");

	for (int i = 10; i > 0; i--) {
		printf("  motor ON %ds\r\n", i);
		HAL_Delay(1000);
	}

	HAL_GPIO_WritePin(AIN1_GPIO_Port, AIN1_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(AIN2_GPIO_Port, AIN2_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(BIN1_GPIO_Port, BIN1_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(BIN2_GPIO_Port, BIN2_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(PWMA_GPIO_Port, PWMA_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(PWMB_GPIO_Port, PWMB_Pin, GPIO_PIN_RESET);

	PwmPins_RestoreAf();
	MotorInit();
	printf("Motor DC test done.\r\n");
}

void MotorControl(int16_t speed)
{
	uint16_t pwm = (uint16_t)abs((int)speed);
	if (pwm > 1000) {
		pwm = 1000;
	}

	MotorState dir_a;
	MotorState dir_b;
	if (speed < 0) {
		dir_a = MOTOR_CW;
		dir_b = MOTOR_CW;
	} else if (speed > 0) {
		dir_a = MOTOR_CCW;
		dir_b = MOTOR_CCW;
	} else {
		MotorAControl(MOTOR_STOP, 0);
		MotorBControl(MOTOR_STOP, 0);
		return;
	}

#if MOTOR_B_INVERT
	/* Left/right motors are usually mirrored on a balance car */
	if (dir_b == MOTOR_CW) {
		dir_b = MOTOR_CCW;
	} else if (dir_b == MOTOR_CCW) {
		dir_b = MOTOR_CW;
	}
#endif

	MotorAControl(dir_a, pwm);
	MotorBControl(dir_b, pwm);
}

void MotorAControl(MotorState state, uint16_t speed)
{
	speed = (speed > 1000) ? 1000 : speed;

	switch (state) {
	case MOTOR_CW:
		HAL_GPIO_WritePin(AIN1_GPIO_Port, AIN1_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(AIN2_GPIO_Port, AIN2_Pin, GPIO_PIN_RESET);
		break;
	case MOTOR_CCW:
		HAL_GPIO_WritePin(AIN1_GPIO_Port, AIN1_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(AIN2_GPIO_Port, AIN2_Pin, GPIO_PIN_SET);
		break;
	case MOTOR_BRAKE:
		HAL_GPIO_WritePin(AIN1_GPIO_Port, AIN1_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(AIN2_GPIO_Port, AIN2_Pin, GPIO_PIN_SET);
		speed = 1000;
		break;
	default:
		HAL_GPIO_WritePin(AIN1_GPIO_Port, AIN1_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(AIN2_GPIO_Port, AIN2_Pin, GPIO_PIN_RESET);
		speed = 0;
		break;
	}

	__HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, speed);
	HAL_GPIO_WritePin(STBY_GPIO_Port, STBY_Pin, GPIO_PIN_SET);
}

void MotorBControl(MotorState state, uint16_t speed)
{
	speed = (speed > 1000) ? 1000 : speed;

	switch (state) {
	case MOTOR_CW:
		HAL_GPIO_WritePin(BIN1_GPIO_Port, BIN1_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(BIN2_GPIO_Port, BIN2_Pin, GPIO_PIN_RESET);
		break;
	case MOTOR_CCW:
		HAL_GPIO_WritePin(BIN1_GPIO_Port, BIN1_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(BIN2_GPIO_Port, BIN2_Pin, GPIO_PIN_SET);
		break;
	case MOTOR_BRAKE:
		HAL_GPIO_WritePin(BIN1_GPIO_Port, BIN1_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(BIN2_GPIO_Port, BIN2_Pin, GPIO_PIN_SET);
		speed = 1000;
		break;
	default:
		HAL_GPIO_WritePin(BIN1_GPIO_Port, BIN1_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(BIN2_GPIO_Port, BIN2_Pin, GPIO_PIN_RESET);
		speed = 0;
		break;
	}

	__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, speed);
	HAL_GPIO_WritePin(STBY_GPIO_Port, STBY_Pin, GPIO_PIN_SET);
}
