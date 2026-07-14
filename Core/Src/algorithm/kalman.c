#include "kalman.h"


// 初始化卡尔曼参数（Pitch角优化配置，自平衡车核心关注）
KalmanFilter pitch_KF = {1, 100, 1000, 0};
KalmanFilter roll_KF = {1, 100, 1000, 0};
KalmanFilter yaw_KF = {5, 200, 1000, 0}; // Yaw角漂移较大，适当调整R值


// 初始化卡尔曼滤波器
void kalman_init(KalmanFilter *kf, int32_t initial_x_hat, int32_t initial_P, int32_t Q, int32_t R) {
    kf->x_hat = initial_x_hat;
    kf->P = initial_P;
    kf->Q = Q;
    kf->R = R;
}

// 执行卡尔曼滤波更新步骤
int32_t kalman_update(KalmanFilter *kf, int32_t measurement) {
    // 预测步骤
    int32_t x_hat_minus = kf->x_hat;
    int32_t P_minus = kf->P + kf->Q;

    // 更新步骤
    int32_t K = (int32_t)(((int64_t)P_minus * Q30) / (P_minus + kf->R));
    kf->x_hat = x_hat_minus + (int32_t)(((int64_t)K * (measurement - x_hat_minus)) / Q30);
    kf->P = (int32_t)(((int64_t)(Q30 - K) * P_minus) / Q30);

    return kf->x_hat;
}
