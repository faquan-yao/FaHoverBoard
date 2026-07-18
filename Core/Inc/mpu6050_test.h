/*
 * mpu6050_test.h
 *
 *  MPU6050 DMP board self-test (UART).
 */

#ifndef INC_MPU6050_TEST_H_
#define INC_MPU6050_TEST_H_

/**
 * Blocking MPU6050 test loop.
 * UART line format: pitch, roll, yaw, gyro_dps
 * Call after MPU6050_DMP_init(). Does not return.
 */
void MPU_SelfTest(void);

#endif /* INC_MPU6050_TEST_H_ */
