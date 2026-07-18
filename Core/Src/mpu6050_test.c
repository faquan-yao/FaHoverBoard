/*
 * mpu6050_test.c
 *
 *  MPU6050 self-test: print pitch, roll, yaw, gyro_dps.
 */
#include "mpu6050_test.h"
#include "mpu6050.h"
#include "main.h"
#include <stdio.h>

void MPU_SelfTest(void)
{
	printf("\r\n==== MPU6050 self-test ====\r\n");
	printf("format: pitch,roll,yaw,gyro_dps\r\n");
	printf("units: deg,deg,deg,deg/s\r\n\r\n");

	while (1) {
		float pitch, roll, yaw, gyro_dps;

		if (MPU6050_DMP_Get_Date(&pitch, &roll, &yaw, &gyro_dps) != 0) {
			HAL_Delay(1);
			continue;
		}

		printf("%.2f,%.2f,%.2f,%.2f\r\n", pitch, roll, yaw, gyro_dps);
		HAL_Delay(100);
	}
}
