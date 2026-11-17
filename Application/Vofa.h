/*
 * Vofa.h
 *
 *  Created on: 2026年11月16日
 *      Author: yaofa
 */

#ifndef VOFA_H_
#define VOFA_H_

#include <stdint.h>

void Vofa_FireWater_Send(uint8_t ch_num, float *val);
void Vofa_JustFloat_Send(uint8_t ch_num, float *val);

void Vofa_Send_IMU(float pitch, float roll, float yaw);
void Vofa_Send_Filt_Data(float raw, float filt);

/*
 * VOFA+ 在线调参。控件命令用 ASCII，%.3f 会被滑块当前值替换，! 为结束符。
 * 串口发送框直接回车（\n）同样有效。
 *
 *   BK=%.3f!     平衡环 Kp
 *   BI=%.3f!     平衡环 Ki（默认 0，写出非 0 才参与运算）
 *   BD=%.3f!     平衡环 Kd
 *   BMAX=%.1f!   平衡环输出限幅，对称
 *   BILIM=%.2f!  平衡环积分限幅
 *   SK=%.3f!     速度环 Kp
 *   SI=%.3f!     速度环 Ki
 *   SD=%.3f!     速度环 Kd
 *   SMAX=%.1f!   速度环输出限幅，对称，单位度，叠加到俯仰目标
 *   SILIM=%.2f!  速度环积分限幅
 *   ST=%.2f!     速度目标，输出轴 RPM
 *   ?            回读当前参数（插入一行文本，JustFloat 波形会闪一下）
 */
void Vofa_PidTune_Init(void);
void Vofa_PidTune_Poll(void);

#endif /* VOFA_H_ */
