/*
 * accel_cal.h
 *
 *  Six-face accelerometer calibration + flash persistence.
 */
#ifndef INC_ACCEL_CAL_H_
#define INC_ACCEL_CAL_H_

#include <stdint.h>

#define ACCEL_CAL_MAGIC  0xACA10001u

typedef struct {
	uint32_t magic;
	float bias_g[3];   /* offset in g */
	float scale[3];    /* scale so (raw_g - bias) * scale ≈ ±1 on that axis */
	uint32_t crc;      /* CRC32 of preceding fields */
} AccelCalData_t;

int AccelCal_Load(AccelCalData_t *out);
int AccelCal_Save(const AccelCalData_t *in);
int AccelCal_IsValid(const AccelCalData_t *cal);

/* Apply stored cal to MPU HW + DMP accel bias (call after sensors/DMP loaded). */
int AccelCal_ApplyToMpu(void);

/* Soft-correct accel in g: out = (in - bias) * scale */
void AccelCal_Correct(const float in_g[3], float out_g[3]);

/**
 * Interactive UART 6-face calibration (blocking).
 * Requires MPU basic sensors ready; disables DMP for raw reads.
 * On success writes flash and applies biases.
 */
void AccelCal_SelfTest(void);

#endif /* INC_ACCEL_CAL_H_ */
