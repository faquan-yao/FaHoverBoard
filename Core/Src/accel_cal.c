/*
 * accel_cal.c
 *
 *  Six-face accelerometer calibration; results stored in last flash page.
 */
#include "accel_cal.h"
#include "flash_storage.h"
#include "inv_mpu.h"
#include "inv_mpu_dmp_motion_driver.h"
#include "main.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stddef.h>

static AccelCalData_t s_cal;
static uint8_t s_cal_loaded;

static uint32_t AccelCal_PayloadCrc(const AccelCalData_t *cal)
{
	/* CRC over all fields except crc itself */
	return Flash_Crc32(cal, (uint32_t)offsetof(AccelCalData_t, crc));
}

int AccelCal_IsValid(const AccelCalData_t *cal)
{
	if (cal == NULL || cal->magic != ACCEL_CAL_MAGIC) {
		return 0;
	}
	if (Flash_Crc32(cal, (uint32_t)offsetof(AccelCalData_t, crc)) != cal->crc) {
		return 0;
	}
	for (int i = 0; i < 3; i++) {
		if (!(cal->scale[i] > 0.5f && cal->scale[i] < 2.0f)) {
			return 0;
		}
		if (fabsf(cal->bias_g[i]) > 0.5f) {
			return 0;
		}
	}
	return 1;
}

int AccelCal_Load(AccelCalData_t *out)
{
	AccelCalData_t tmp;

	if (Flash_ReadCal(&tmp, sizeof(tmp)) != 0) {
		return -1;
	}
	if (!AccelCal_IsValid(&tmp)) {
		return -1;
	}
	s_cal = tmp;
	s_cal_loaded = 1;
	if (out) {
		*out = tmp;
	}
	return 0;
}

int AccelCal_Save(const AccelCalData_t *in)
{
	AccelCalData_t tmp;

	if (in == NULL) {
		return -1;
	}
	tmp = *in;
	tmp.magic = ACCEL_CAL_MAGIC;
	tmp.crc = AccelCal_PayloadCrc(&tmp);
	if (!AccelCal_IsValid(&tmp)) {
		return -1;
	}
	if (Flash_WriteCal(&tmp, sizeof(tmp)) != 0) {
		return -1;
	}
	s_cal = tmp;
	s_cal_loaded = 1;
	return 0;
}

void AccelCal_Correct(const float in_g[3], float out_g[3])
{
	if (!s_cal_loaded) {
		if (AccelCal_Load(NULL) != 0) {
			out_g[0] = in_g[0];
			out_g[1] = in_g[1];
			out_g[2] = in_g[2];
			return;
		}
	}
	out_g[0] = (in_g[0] - s_cal.bias_g[0]) * s_cal.scale[0];
	out_g[1] = (in_g[1] - s_cal.bias_g[1]) * s_cal.scale[1];
	out_g[2] = (in_g[2] - s_cal.bias_g[2]) * s_cal.scale[2];
}

int AccelCal_ApplyToMpu(void)
{
	unsigned short sens = 16384;
	long bias_8g[3];
	long bias_dmp[3];

	if (!s_cal_loaded) {
		if (AccelCal_Load(NULL) != 0) {
			return -1;
		}
	}

	mpu_get_accel_sens(&sens);

	/*
	 * HW bias regs expect LSB in ±8g format (4096 LSB/g).
	 * DMP bias path matches run_self_test: value * accel_sens.
	 */
	for (int i = 0; i < 3; i++) {
		bias_8g[i] = (long)(s_cal.bias_g[i] * 4096.0f);
		bias_dmp[i] = (long)(s_cal.bias_g[i] * (float)sens);
	}

	if (mpu_set_accel_bias_6050_reg(bias_8g) != 0) {
		printf("AccelCal: HW bias write failed\r\n");
		/* still try DMP */
	}
	if (dmp_set_accel_bias(bias_dmp) != 0) {
		printf("AccelCal: DMP bias write failed\r\n");
		return -1;
	}

	printf("AccelCal applied: bias_g=[%.4f %.4f %.4f] scale=[%.4f %.4f %.4f]\r\n",
	       s_cal.bias_g[0], s_cal.bias_g[1], s_cal.bias_g[2],
	       s_cal.scale[0], s_cal.scale[1], s_cal.scale[2]);
	return 0;
}

static int AccelCal_ReadAvgG(float avg_g[3], int samples, int period_ms)
{
	unsigned short sens = 16384;
	float sum[3] = {0.0f, 0.0f, 0.0f};
	int got = 0;

	mpu_get_accel_sens(&sens);
	if (sens == 0) {
		sens = 16384;
	}

	while (got < samples) {
		short raw[3];
		if (mpu_get_accel_reg(raw, 0) != 0) {
			HAL_Delay(period_ms);
			continue;
		}
		sum[0] += (float)raw[0] / (float)sens;
		sum[1] += (float)raw[1] / (float)sens;
		sum[2] += (float)raw[2] / (float)sens;
		got++;
		HAL_Delay(period_ms);
	}

	avg_g[0] = sum[0] / (float)samples;
	avg_g[1] = sum[1] / (float)samples;
	avg_g[2] = sum[2] / (float)samples;
	return 0;
}

static void AccelCal_Countdown(const char *face_name)
{
	printf("\r\n>>> Place device: %s\r\n", face_name);
	printf("    Keep still on a flat surface.\r\n");
	for (int i = 5; i >= 1; i--) {
		printf("    sampling in %d s...\r\n", i);
		HAL_Delay(1000);
	}
	printf("    sampling...\r\n");
}

static int AccelCal_CollectFace(const char *name, float avg_g[3])
{
	AccelCal_Countdown(name);
	if (AccelCal_ReadAvgG(avg_g, 200, 10) != 0) {
		return -1;
	}
	printf("    avg = [%.4f, %.4f, %.4f] g\r\n",
	       avg_g[0], avg_g[1], avg_g[2]);
	return 0;
}

void AccelCal_SelfTest(void)
{
	float zp[3], zn[3], yp[3], yn[3], xp[3], xn[3];
	AccelCalData_t cal;

	printf("\r\n==== Accel 6-face calibration ====\r\n");
	printf("Flash page @ 0x%08lX\r\n", (unsigned long)FLASH_CAL_PAGE_ADDR);
	printf("Follow prompts: rest each face flat, wait for sample.\r\n");

	/* Prefer raw register reads without DMP FIFO contention */
	(void)mpu_set_dmp_state(0);
	(void)mpu_configure_fifo(0);
	(void)mpu_set_sensors(INV_XYZ_GYRO | INV_XYZ_ACCEL);
	(void)mpu_set_sample_rate(100);
	HAL_Delay(50);

	if (AccelCal_CollectFace("Z-up   (+Z, az~+1g)", zp) != 0) goto fail;
	if (AccelCal_CollectFace("Z-down (-Z, az~-1g)", zn) != 0) goto fail;
	if (AccelCal_CollectFace("Y-up   (+Y, ay~+1g)", yp) != 0) goto fail;
	if (AccelCal_CollectFace("Y-down (-Y, ay~-1g)", yn) != 0) goto fail;
	if (AccelCal_CollectFace("X-up   (+X, ax~+1g)", xp) != 0) goto fail;
	if (AccelCal_CollectFace("X-down (-X, ax~-1g)", xn) != 0) goto fail;

	memset(&cal, 0, sizeof(cal));
	cal.magic = ACCEL_CAL_MAGIC;
	cal.bias_g[0] = 0.5f * (xp[0] + xn[0]);
	cal.bias_g[1] = 0.5f * (yp[1] + yn[1]);
	cal.bias_g[2] = 0.5f * (zp[2] + zn[2]);

	{
		float dx = xp[0] - xn[0];
		float dy = yp[1] - yn[1];
		float dz = zp[2] - zn[2];
		if (fabsf(dx) < 0.5f || fabsf(dy) < 0.5f || fabsf(dz) < 0.5f) {
			printf("ERROR: face span too small (dx=%.3f dy=%.3f dz=%.3f)\r\n",
			       dx, dy, dz);
			goto fail;
		}
		cal.scale[0] = 2.0f / dx;
		cal.scale[1] = 2.0f / dy;
		cal.scale[2] = 2.0f / dz;
	}

	printf("\r\nComputed bias_g = [%.4f %.4f %.4f]\r\n",
	       cal.bias_g[0], cal.bias_g[1], cal.bias_g[2]);
	printf("Computed scale  = [%.4f %.4f %.4f]\r\n",
	       cal.scale[0], cal.scale[1], cal.scale[2]);

	if (AccelCal_Save(&cal) != 0) {
		printf("ERROR: flash save failed or result out of range\r\n");
		goto fail;
	}
	printf("Saved to flash OK.\r\n");

	/* Reload DMP path briefly to push bias (firmware already loaded earlier) */
	(void)mpu_configure_fifo(INV_XYZ_GYRO | INV_XYZ_ACCEL);
	(void)dmp_enable_feature(DMP_FEATURE_6X_LP_QUAT |
				 DMP_FEATURE_SEND_CAL_GYRO |
				 DMP_FEATURE_GYRO_CAL);
	(void)dmp_set_fifo_rate(100);
	if (AccelCal_ApplyToMpu() != 0) {
		printf("WARNING: apply to MPU incomplete\r\n");
	}
	(void)mpu_set_dmp_state(1);

	printf("==== Accel calibration done. Power-cycle or set RUN_ACCEL_CAL=0 ====\r\n");
	while (1) {
		HAL_Delay(1000);
	}

fail:
	printf("==== Accel calibration FAILED ====\r\n");
	while (1) {
		HAL_Delay(1000);
	}
}
