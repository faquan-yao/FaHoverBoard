#ifndef __MY_MPU6050_H__
#define __MY_MPU6050_H__

#include "stdint.h"
#include "fixed_math.h"

#define ERROR_MPU_INIT      -1
#define ERROR_SET_SENSOR    -2
#define ERROR_CONFIG_FIFO   -3
#define ERROR_SET_RATE      -4
#define ERROR_LOAD_MOTION_DRIVER    -5
#define ERROR_SET_ORIENTATION       -6
#define ERROR_ENABLE_FEATURE        -7
#define ERROR_SET_FIFO_RATE         -8
#define ERROR_SELF_TEST             -9
#define ERROR_DMP_STATE             -10

#define DEFAULT_MPU_HZ  100

/*
 * MPU6050_DMP_Get_Date returns pitch/roll/yaw in Q30 radians.
 * Convert to float radians: angle_rad = raw / 1073741824.0f
 * Convert to centi-degrees: q30_to_centi_deg(raw)  (0.01 deg units)
 */

int MPU6050_DMP_init(void);
int MPU6050_DMP_Get_Date(int32_t *pitch, int32_t *roll, int32_t *yaw);

#endif
