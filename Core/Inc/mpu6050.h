#ifndef __MY_MPU6050_H__
#define __MY_MPU6050_H__

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
#define Q30  1073741824.0f

/* Gyro axis used as pitch rate after DMP orientation. Swap 0/1 if damping wrong. */
#ifndef PITCH_GYRO_AXIS
#define PITCH_GYRO_AXIS  1
#endif

int MPU6050_DMP_init(void);

/* pitch/roll/yaw in deg; gyro_pitch_dps is angular rate for pitch damping */
int MPU6050_DMP_Get_Date(float *pitch, float *roll, float *yaw, float *gyro_pitch_dps);

#endif
