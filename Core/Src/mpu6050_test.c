/*
 * mpu6050_test.c
 *
 *  MPU6050 self-test: print pitch, roll, yaw, gyro_dps.
 */
#include "mpu6050_test.h"
#include "mpu6050.h"
#include "main.h"
#include "log.h"

void MPU_SelfTest(void)
{
	Log(LOG_INFO, "\r\n==== MPU6050 self-test ====\r\n");
	Log(LOG_INFO, "format: pitch,roll,yaw,gyro_dps\r\n");
	Log(LOG_INFO, "units: deg,deg,deg,deg/s\r\n\r\n");

	while (1) {
		float pitch, roll, yaw, gyro_dps;

		if (MPU6050_DMP_Get_Date(&pitch, &roll, &yaw, &gyro_dps) != 0) {
			HAL_Delay(1);
			continue;
		}

		Log(LOG_INFO, "%.2f,%.2f,%.2f,%.2f\r\n", pitch, roll, yaw, gyro_dps);
		HAL_Delay(100);
	}
}
