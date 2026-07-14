/*
 * kalman.h
 *
 *  Created on: Dec 22, 2025
 *      Author: yaofa
 */

#ifndef INC_KALMAN_H_
#define INC_KALMAN_H_

#include "stdint.h"
#include "fixed_math.h"

// 卡尔曼滤波结构体（单轴，可复制为roll/pitch/yaw分别使用）
typedef struct {
    int32_t x_hat;    // 估计值
    int32_t P;        // 估计误差协方差
    int32_t Q;        // 过程噪声协方差
    int32_t R;        // 测量噪声协方差
} KalmanFilter;


// 初始化卡尔曼参数（Pitch角优化配置，自平衡车核心关注）
extern KalmanFilter pitch_KF;
extern KalmanFilter roll_KF;
extern KalmanFilter yaw_KF;

void kalman_init(KalmanFilter *kf, int32_t initial_x_hat, int32_t initial_P, int32_t Q, int32_t R);
int32_t kalman_update(KalmanFilter *kf, int32_t measurement);
#endif /* INC_KALMAN_H_ */
