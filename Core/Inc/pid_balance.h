/*
 * pid_balance.h
 *
 *  Created on: Dec 23, 2025
 *      Author: yaofa
 */

#ifndef INC_PID_BALANCE_H_
#define INC_PID_BALANCE_H_

/* Manual upright pitch (deg): hold car still, read serial p=, put that value here. */
#ifndef ANGLE_MECH_ZERO
#define ANGLE_MECH_ZERO  0.58f
#endif

/* Manual gyro bias (°/s) for pitch-rate D term. Usually 0 if DMP GYRO_CAL is OK. */
#ifndef GYRO_BIAS
#define GYRO_BIAS        0.0f
#endif

/* First-order LPF on gyro before D-term / recover. Weight on new sample.
 * DMP ~100 Hz: 0.3 ≈ 7 Hz. Raise if damping feels laggy, lower if noisy. */
#ifndef GYRO_LPF_ALPHA
#define GYRO_LPF_ALPHA   0.4f
#endif

/* Speed→angle trim sign. Flip to +1.0f if car accelerates the wrong way. */
#ifndef SPEED_TRIM_SIGN
#define SPEED_TRIM_SIGN  (1.0f)
#endif

/* +1: duty = +gain*bal; -1: invert motor command.
 * 2026-08-20_17-53 CSV: MOTOR_CMD_SIGN=-1 left pitch stuck ~-1° then tipped — revert to +1. */
#ifndef MOTOR_CMD_SIGN
#define MOTOR_CMD_SIGN   (1.0f)
#endif

/* Speed-loop target (output-shaft RPM). */
extern float speed_target_rpm;

/* Gyro D-term sign. Flip to -1.0f if damping makes it worse. */
#ifndef GYRO_D_SIGN
#define GYRO_D_SIGN      (-1.0f)
#endif

/* Fall cut-out vs mechanical zero. Recover uses hysteresis + hold time. */
#ifndef FALL_ANGLE_DEG
#define FALL_ANGLE_DEG     30.0f
#endif
#ifndef RECOVER_ANGLE_DEG
#define RECOVER_ANGLE_DEG  10.0f
#endif
#ifndef RECOVER_GYRO_DPS
#define RECOVER_GYRO_DPS   20.0f
#endif
#ifndef RECOVER_HOLD_MS
#define RECOVER_HOLD_MS    300U
#endif
#ifndef IMU_FAIL_KILL_MS
#define IMU_FAIL_KILL_MS   50U
#endif

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
void PID_Init(void);
float PID_Calc(PID_TypeDef *pid);
float PID_BalanceGyro(PID_TypeDef *pid, float gyro_dps);

#endif /* INC_PID_BALANCE_H_ */
