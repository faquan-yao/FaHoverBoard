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

#endif /* VOFA_H_ */
