#include "pid.h"

/**
 * @brief  PID初始化函数
 * @param  pid: PID结构体指针
 * @param  Kp: 比例系数
 * @param  Ki: 积分系数
 * @param  Kd: 微分系数
 * @param  output_max: 输出最大值
 * @param  output_min: 输出最小值
 * @param  filter_k: 微分滤波系数（建议0.2~0.8）
 */
void PID_Init(PID_HandleTypeDef *pid, float Kp, float Ki, float Kd,
              float output_max, float output_min, float filter_k) {
    // 初始化PID参数
    pid->Kp = Kp;
    pid->Ki = Ki;
    pid->Kd = Kd;
    
    // 初始化误差和输出
    pid->err = 0.0f;
    pid->err_prev1 = 0.0f;
    pid->err_prev2 = 0.0f;
    pid->output = 0.0f;
    pid->output_prev = 0.0f;
    pid->integral = 0.0f;
    
    // 初始化配置参数
    pid->output_max = output_max;
    pid->output_min = output_min;
    pid->integral_max = output_max * 0.5f; // 积分限幅设为输出限幅的50%
    pid->integral_min = output_min * 0.5f;
    pid->filter_k = filter_k;
    pid->diff_filter = 0.0f;
}

/**
 * @brief  PID计算核心函数
 * @param  pid: PID结构体指针
 * @param  set: 设定值（目标值）
 * @param  feedback: 反馈值（实际测量值）
 * @retval PID输出值
 */
float PID_Calculate(PID_HandleTypeDef *pid, float set, float feedback) {
    // 1. 计算当前误差
    pid->err = set - feedback;
    
#if (PID_MODE_INCREMENT == 0)
    // ========== 位置式PID ==========
    // 1.1 积分项计算 + 积分限幅（抗积分饱和）
    pid->integral += pid->Ki * pid->err;
    // 积分限幅
    if (pid->integral > pid->integral_max) {
        pid->integral = pid->integral_max;
    } else if (pid->integral < pid->integral_min) {
        pid->integral = pid->integral_min;
    }
    
    // 1.2 微分项计算 + 一阶低通滤波（抑制噪声）
    float diff = pid->err - pid->err_prev1;
    pid->diff_filter = pid->filter_k * diff + (1 - pid->filter_k) * pid->diff_filter;
    float derivative = pid->Kd * pid->diff_filter;
    
    // 1.3 位置式PID输出计算
    pid->output = pid->Kp * pid->err + pid->integral + derivative;
    
#else
    // ========== 增量式PID ==========
    // 2.1 增量计算 (Δu = Kp*(e - e1) + Ki*e + Kd*(e - 2e1 + e2))
    float delta_u = pid->Kp * (pid->err - pid->err_prev1) 
                  + pid->Ki * pid->err 
                  + pid->Kd * (pid->err - 2 * pid->err_prev1 + pid->err_prev2);
    
    // 2.2 增量式输出（累加）
    pid->output = pid->output_prev + delta_u;
    
#endif
    
    // 3. 输出限幅（适配执行器范围）
    if (pid->output > pid->output_max) {
        pid->output = pid->output_max;
    } else if (pid->output < pid->output_min) {
        pid->output = pid->output_min;
    }
    
    // 4. 更新误差缓存（为下一次计算准备）
    pid->err_prev2 = pid->err_prev1;
    pid->err_prev1 = pid->err;
    pid->output_prev = pid->output;
    
    return pid->output;
}

/**
 * @brief  PID参数重置函数（清空积分、误差）
 * @param  pid: PID结构体指针
 */
void PID_Reset(PID_HandleTypeDef *pid) {
    pid->err = 0.0f;
    pid->err_prev1 = 0.0f;
    pid->err_prev2 = 0.0f;
    pid->output = 0.0f;
    pid->output_prev = 0.0f;
    pid->integral = 0.0f;
    pid->diff_filter = 0.0f;
}