/*
 * tb6612fng.h
 *
 *  Created on: Nov 16, 2025
 *      Author: yaofa
 */

#ifndef INC_TB6612FNG_H_
#define INC_TB6612FNG_H_

// 电机动作控制枚举
typedef enum {
    MOTOR_STOP = 0,
    MOTOR_CW,    // 顺时针
    MOTOR_CCW,   // 逆时针
    MOTOR_BRAKE  // 刹车
} MotorState;

void MotorInit();
void MotorControl(int16_t speed);
void MotorAControl(MotorState state, uint16_t speed);
void MotorBControl(MotorState state, uint16_t speed);

#endif /* INC_TB6612FNG_H_ */
