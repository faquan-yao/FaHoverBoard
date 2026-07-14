
#ifndef INCLUDE_PID_H
#define INCLUDE_PID_H

// PID控制模式选择：0-位置式PID，1-增量式PID
#define PID_MODE_INCREMENT  1

// PID结构体定义（通用型，适配多通道）
typedef struct {
    // 1. PID核心参数
    float Kp;          // 比例系数
    float Ki;          // 积分系数
    float Kd;          // 微分系数
    
    // 2. 误差变量
    float err;         // 当前误差 (set - feedback)
    float err_prev1;   // 上一次误差
    float err_prev2;   // 上两次误差
    
    // 3. 输出变量
    float output;      // PID输出值
    float output_prev; // 上一次输出（增量式用）
    float integral;    // 积分项（位置式用）
    
    // 4. 配置参数
    float output_max;  // 输出最大值（限幅）
    float output_min;  // 输出最小值（限幅）
    float integral_max;// 积分限幅最大值（抗积分饱和）
    float integral_min;// 积分限幅最小值
    float filter_k;    // 微分滤波系数（0~1，越小滤波越强）
    
    // 5. 微分滤波缓存
    float diff_filter; // 微分滤波后的值
} PID_HandleTypeDef;

void PID_Init(PID_HandleTypeDef *pid, float Kp, float Ki, float Kd,
              float output_max, float output_min, float filter_k);

float PID_Calculate(PID_HandleTypeDef *pid, float set, float feedback);

void PID_Reset(PID_HandleTypeDef *pid);
#endif