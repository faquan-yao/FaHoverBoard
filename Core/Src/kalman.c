#include "kalman.h"


// 初始化卡尔曼参数（Pitch角优化配置，自平衡车核心关注）
KalmanFilter pitch_KF = {0.001f, 0.1f, 1.0f, 0.0f};
KalmanFilter roll_KF = {0.001f, 0.1f, 1.0f, 0.0f};
KalmanFilter yaw_KF = {0.005f, 0.2f, 1.0f, 0.0f}; // Yaw角漂移较大，适当调整R值

// 卡尔曼滤波更新：输入测量值，返回滤波后的值
float kalman_update(KalmanFilter *kf, float measure) {
    // 1. 预测阶段：X = 上一时刻估计值（姿态角变化缓慢，预测值=上一估计值）
    kf->P = kf->P + kf->Q; // 估计误差协方差更新

    // 2. 更新阶段：计算卡尔曼增益
    float K = kf->P / (kf->P + kf->R);

    // 3. 修正估计值
    kf->X = kf->X + K * (measure - kf->X);

    // 4. 更新估计误差协方差
    kf->P = (1 - K) * kf->P;

    return kf->X;
}
