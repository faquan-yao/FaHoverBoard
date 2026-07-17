/*
 * pid_balance.c
 *
 *  Created on: Dec 23, 2025
 *      Author: yaofa
 */
#include "pid_balance.h"


// 平衡环PID初始化（核心参数需根据实际硬件调试）
PID_TypeDef BalancePID = {
    .Kp = 8.0f,    // 比例系数（示例值，需调试）
    .Ki = 0.2f,    // 积分系数（示例值，需调试）
    .Kd = 1.5f,    // 微分系数（示例值，需调试）
    .MaxOut = 80.0f,// PWM最大输出占空比（%）
    .MinOut = -80.0f,// PWM最小输出占空比（%）
    .MaxI = 50.0f, // 积分限幅
    .SetVal = 0.0f, // 平衡目标角度（水平0°）
    .NowVal = 0.0f,
    .Err = 0.0f,
    .LastErr = 0.0f,
    .PreErr = 0.0f,
    .Pout = 0.0f,
    .Iout = 0.0f,
    .Dout = 0.0f,
    .Out = 0.0f
};

PID_TypeDef SpeedPID = {
	    .Kp = 8.0f,    // 比例系数（示例值，需调试）
	    .Ki = 0.2f,    // 积分系数（示例值，需调试）
	    .Kd = 1.5f,    // 微分系数（示例值，需调试）
	    .MaxOut = 80.0f,// PWM最大输出占空比（%）
	    .MinOut = -80.0f,// PWM最小输出占空比（%）
	    .MaxI = 50.0f, // 积分限幅
	    .SetVal = 0.0f, // 平衡目标角度（水平0°）
	    .NowVal = 0.0f,
	    .Err = 0.0f,
	    .LastErr = 0.0f,
	    .PreErr = 0.0f,
	    .Pout = 0.0f,
	    .Iout = 0.0f,
	    .Dout = 0.0f,
	    .Out = 0.0f
	};

// PID参数初始化（可外部调用修改参数）
void PID_Init(void)
{
    // 重置PID中间变量
    BalancePID.Err = 0.0f;
    BalancePID.LastErr = 0.0f;
    BalancePID.PreErr = 0.0f;
    BalancePID.Iout = 0.0f;
    BalancePID.Out = 0.0f;

    SpeedPID.Err = 0.0f;
    SpeedPID.LastErr = 0.0f;
    SpeedPID.PreErr = 0.0f;
    SpeedPID.Iout = 0.0f;
    SpeedPID.Out = 0.0f;
}

// PID核心计算函数
float PID_Calc(PID_TypeDef *pid)
{
    // 1. 计算当前误差（设定值 - 当前值）
    pid->Err = pid->SetVal - pid->NowVal;

    // 2. 比例项
    pid->Pout = pid->Kp * pid->Err;

    // 3. 积分项（积分限幅防止饱和）
    pid->Iout += pid->Ki * pid->Err;
    if (pid->Iout > pid->MaxI) pid->Iout = pid->MaxI;
    if (pid->Iout < -pid->MaxI) pid->Iout = -pid->MaxI;

    // 4. 微分项（二阶微分，减少抖振）
    pid->Dout = pid->Kd * (pid->Err - 2 * pid->LastErr + pid->PreErr);

    // 5. 总输出（限幅）
    pid->Out = pid->Pout + pid->Iout + pid->Dout;
    if (pid->Out > pid->MaxOut) pid->Out = pid->MaxOut;
    if (pid->Out < pid->MinOut) pid->Out = pid->MinOut;

    // 6. 更新误差历史
    pid->PreErr = pid->LastErr;
    pid->LastErr = pid->Err;

    return pid->Out;
}
