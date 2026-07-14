/*
 * encoder.h
 *
 *  Created on: Jan 18, 2026
 *      Author: yaofa
 */

#ifndef BSP_INC_ENCODER_H_
#define BSP_INC_ENCODER_H_
#include <stdint.h>
#include "tim.h"

enum MEASURED_TYPE {
	M = 0,
	T,
};

typedef struct {
	enum MEASURED_TYPE type;
	int16_t measuredValue;
	uint32_t timestamp;
} EncoderData;
extern volatile int32_t encoderA_count;
extern volatile int32_t encoderB_count;
extern volatile enum MEASURED_TYPE encoderA_type;
extern volatile enum MEASURED_TYPE encoderB_type;

void Enc_init();

#endif /* BSP_INC_ENCODER_H_ */
