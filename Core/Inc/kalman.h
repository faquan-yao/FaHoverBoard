/*
 * kalman.h
 *
 *  Created on: Dec 22, 2025
 *      Author: yaofa
 */

#ifndef INC_KALMAN_H_
#define INC_KALMAN_H_


// 卡尔曼滤波结构体（单轴，可复制为roll/pitch/yaw分别使用）
typedef struct {
    float Q; // 过程噪声协方差
    float R; // 测量噪声协方差
    float P; // 估计误差协方差
    float X; // 滤波后的值（估计值）
} KalmanFilter;

// 初始化卡尔曼参数（Pitch角优化配置，自平衡车核心关注）
extern KalmanFilter pitch_KF;
extern KalmanFilter roll_KF;
extern KalmanFilter yaw_KF;

float kalman_update(KalmanFilter *kf, float measure);
#endif /* INC_KALMAN_H_ */
