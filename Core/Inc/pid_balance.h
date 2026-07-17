/*
 * pid_balance.h
 *
 *  Created on: Dec 23, 2025
 *      Author: yaofa
 */

#ifndef INC_PID_BALANCE_H_
#define INC_PID_BALANCE_H_

// PID参数结构体
typedef struct PID_TypeDef_Struct {
    // 基本参数
    float Kp;    // 比例系数
    float Ki;    // 积分系数
    float Kd;    // 微分系数

    // 控制限幅
    float MaxOut; // 输出最大值
    float MinOut; // 输出最小值
    float MaxI;   // 积分限幅

    // 中间变量
    float SetVal; // 设定值（平衡角度，如0°）
    float NowVal; // 当前值（欧拉角pitch）

    float Err;    // 当前误差
    float LastErr;// 上一次误差
    float PreErr; // 上上次误差
    float Pout;   // 比例输出
    float Iout;   // 积分输出
    float Dout;   // 微分输出
    float Out;    // 总输出（PWM占空比计算基础）
} PID_TypeDef;

// 全局变量声明
extern PID_TypeDef BalancePID; // 平衡环PID
extern PID_TypeDef SpeedPID; // 速度环PID

// 函数声明
void PID_Init(void);            // PID参数初始化
float PID_Calc(PID_TypeDef *pid);// PID计算

#endif /* INC_PID_BALANCE_H_ */
