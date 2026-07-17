#include <stdint.h>
#include <tb6612fng.h>
#include "gpio.h"
#include "main.h"
#include "tim.h"
#include "stdlib.h"


void MotorInit() {
	  HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_1);  // 启动PWM输出
	  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);  // 启动PWM输出
}

void MotorControl(int16_t speed) {
	if(speed < 0) {
		MotorAControl(MOTOR_CW, abs(speed));
		MotorBControl(MOTOR_CW, abs(speed));
	} else if (speed > 0) {
		MotorAControl(MOTOR_CCW, abs(speed));
		MotorBControl(MOTOR_CCW, abs(speed));
	}
}

void MotorAControl(MotorState state, uint16_t speed) {
    // 速度限幅(0-1000对应0%-100%)
    speed = (speed > 1000) ? 1000 : speed;

    switch(state) {
        case MOTOR_CW:
        	HAL_GPIO_WritePin(AIN1_GPIO_Port, AIN1_Pin, GPIO_PIN_SET);  // AIN1=1
            HAL_GPIO_WritePin(AIN2_GPIO_Port, AIN2_Pin, GPIO_PIN_RESET); // AIN2=0
            break;
        case MOTOR_CCW:
        	HAL_GPIO_WritePin(AIN1_GPIO_Port, AIN1_Pin, GPIO_PIN_RESET);  // AIN1=0
            HAL_GPIO_WritePin(AIN2_GPIO_Port, AIN2_Pin, GPIO_PIN_SET); // AIN2=1
            break;
        case MOTOR_BRAKE:
        	HAL_GPIO_WritePin(AIN1_GPIO_Port, AIN1_Pin, GPIO_PIN_SET);  // AIN1=1
            HAL_GPIO_WritePin(AIN2_GPIO_Port, AIN2_Pin, GPIO_PIN_SET); // AIN2=1
            speed = 1000; // 全占空比刹车
            break;
        default:  // STOP
        	HAL_GPIO_WritePin(AIN1_GPIO_Port, AIN1_Pin, GPIO_PIN_RESET);  // AIN1=0
            HAL_GPIO_WritePin(AIN2_GPIO_Port, AIN2_Pin, GPIO_PIN_RESET); // AIN2=0
            speed = 0;
    }

    __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, speed);  // 更新PWM占空比
    HAL_GPIO_WritePin(STBY_GPIO_Port, STBY_Pin, GPIO_PIN_SET);
}

void MotorBControl(MotorState state, uint16_t speed) {
    // 速度限幅(0-1000对应0%-100%)
    speed = (speed > 1000) ? 1000 : speed;

    switch(state) {
        case MOTOR_CW:
        	HAL_GPIO_WritePin(BIN1_GPIO_Port, BIN1_Pin, GPIO_PIN_SET);  // AIN1=1
            HAL_GPIO_WritePin(BIN2_GPIO_Port, BIN2_Pin, GPIO_PIN_RESET); // AIN2=0
            break;
        case MOTOR_CCW:
        	HAL_GPIO_WritePin(BIN1_GPIO_Port, BIN1_Pin, GPIO_PIN_RESET);  // AIN1=0
            HAL_GPIO_WritePin(BIN2_GPIO_Port, BIN2_Pin, GPIO_PIN_SET); // AIN2=1
            break;
        case MOTOR_BRAKE:
        	HAL_GPIO_WritePin(BIN1_GPIO_Port, BIN1_Pin, GPIO_PIN_SET);  // AIN1=1
            HAL_GPIO_WritePin(BIN2_GPIO_Port, BIN2_Pin, GPIO_PIN_SET); // AIN2=1
            speed = 1000; // 全占空比刹车
            break;
        default:  // STOP
        	HAL_GPIO_WritePin(BIN1_GPIO_Port, BIN1_Pin, GPIO_PIN_RESET);  // AIN1=0
            HAL_GPIO_WritePin(BIN2_GPIO_Port, BIN2_Pin, GPIO_PIN_RESET); // AIN2=0
            speed = 0;
    }

    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, speed);  // 更新PWM占空比
    HAL_GPIO_WritePin(STBY_GPIO_Port, STBY_Pin, GPIO_PIN_SET);
}

