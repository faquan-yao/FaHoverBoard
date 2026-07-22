/*
 * encoder_test.c
 *
 *  Motor-driven encoder self-test (3 phases).
 *  Default duty = 25% of PWM full scale (0..1000 → 250).
 *  MotorA = left (TIM4), MotorB = right (TIM1).
 */
#include "encoder_test.h"
#include "encoder.h"
#include "hardware_cfg.h"
#include "tb6612fng.h"
#include "main.h"
#include <stdio.h>
#include <stdlib.h>

/* PWM compare uses 0..1000 in MotorA/BControl */
#ifndef ENC_TEST_DUTY_PCT
#define ENC_TEST_DUTY_PCT  25U
#endif
#define ENC_TEST_PWM       ((uint16_t)((1000U * ENC_TEST_DUTY_PCT) / 100U))

/* Match MotorControl(speed>0): CCW = forward */
#define ENC_DIR_FWD  MOTOR_CCW
#define ENC_DIR_REV  MOTOR_CW

#define ENC_TEST_PRINT_MS     100U
#define ENC_TEST_PHASE_GAP_MS 1000U
#define ENC_TEST_TIMEOUT_MS   30000U

static const char *ENC_DirStr(float rpm)
{
	if (rpm > 1.0f) {
		return "FWD";
	}
	if (rpm < -1.0f) {
		return "REV";
	}
	return "STOP";
}

static MotorState ENC_MaybeInvertB(MotorState dir)
{
#if MOTOR_B_INVERT
	if (dir == MOTOR_CW) {
		return MOTOR_CCW;
	}
	if (dir == MOTOR_CCW) {
		return MOTOR_CW;
	}
#endif
	return dir;
}

static void ENC_MotorsStop(void)
{
	MotorAControl(MOTOR_STOP, 0);
	MotorBControl(MOTOR_STOP, 0);
}

static void ENC_MotorsDrive(MotorState left_dir, MotorState right_dir, uint16_t pwm)
{
	MotorAControl(left_dir, pwm);
	MotorBControl(ENC_MaybeInvertB(right_dir), pwm);
}

static void ENC_PrintStatus(const char *tag)
{
	int32_t total_l, total_r;
	int32_t diff_l, diff_r;
	uint32_t cnt_l, cnt_r;
	float rpm_l, rpm_r;

	ENC_GetTotals(&total_l, &total_r);
	ENC_GetPulseDiffs(&diff_l, &diff_r);
	ENC_GetRawCnt(&cnt_l, &cnt_r);
	ENC_GetSpeeds(&rpm_l, &rpm_r);

	float rev_l = (float)total_l / (float)ENC_PULSES_PER_OUT_REV;
	float rev_r = (float)total_r / (float)ENC_PULSES_PER_OUT_REV;

	printf("[%s] L: cnt=%lu total=%ld d=%ld rev=%+.3f %s rpm=%+.1f | "
	       "R: cnt=%lu total=%ld d=%ld rev=%+.3f %s rpm=%+.1f\r\n",
	       tag,
	       (unsigned long)cnt_l, (long)total_l, (long)diff_l,
	       rev_l, ENC_DirStr(rpm_l), rpm_l,
	       (unsigned long)cnt_r, (long)total_r, (long)diff_r,
	       rev_r, ENC_DirStr(rpm_r), rpm_r);
}

/*
 * Drive until each enabled wheel travels |delta| >= target pulses from start.
 * left_en/right_en: 0 = keep stopped for that side.
 */
static int ENC_DriveUntilPulses(const char *tag,
				MotorState left_dir, MotorState right_dir,
				int left_en, int right_en,
				int32_t target_pulses)
{
	int32_t start_l, start_r;
	int left_done = left_en ? 0 : 1;
	int right_done = right_en ? 0 : 1;
	uint32_t elapsed = 0;

	ENC_ResetTotals();
	HAL_Delay(30);
	ENC_GetTotals(&start_l, &start_r);

	printf("%s start: target=%ld pulses/out-rev duty=%u%% pwm=%u\r\n",
	       tag, (long)target_pulses, (unsigned)ENC_TEST_DUTY_PCT,
	       (unsigned)ENC_TEST_PWM);

	while (!(left_done && right_done)) {
		int32_t total_l, total_r;
		MotorState ldir = MOTOR_STOP;
		MotorState rdir = MOTOR_STOP;
		uint16_t lpwm = 0;
		uint16_t rpwm = 0;

		ENC_GetTotals(&total_l, &total_r);

		if (left_en && !left_done) {
			if (labs((long)(total_l - start_l)) >= (long)target_pulses) {
				left_done = 1;
				printf("%s left done delta=%ld\r\n",
				       tag, (long)(total_l - start_l));
			} else {
				ldir = left_dir;
				lpwm = ENC_TEST_PWM;
			}
		}
		if (right_en && !right_done) {
			if (labs((long)(total_r - start_r)) >= (long)target_pulses) {
				right_done = 1;
				printf("%s right done delta=%ld\r\n",
				       tag, (long)(total_r - start_r));
			} else {
				rdir = right_dir;
				rpwm = ENC_TEST_PWM;
			}
		}

		MotorAControl(ldir, lpwm);
		MotorBControl(ENC_MaybeInvertB(rdir), rpwm);

		ENC_PrintStatus(tag);
		HAL_Delay(ENC_TEST_PRINT_MS);
		elapsed += ENC_TEST_PRINT_MS;

		if (elapsed >= ENC_TEST_TIMEOUT_MS) {
			ENC_MotorsStop();
			printf("%s TIMEOUT after %lums (left_done=%d right_done=%d)\r\n",
			       tag, (unsigned long)elapsed, left_done, right_done);
			return -1;
		}
	}

	ENC_MotorsStop();
	ENC_PrintStatus(tag);
	printf("%s OK\r\n\r\n", tag);
	return 0;
}

void Enc_SelfTest(void)
{
	printf("\r\n==== Encoder self-test (motor-driven) ====\r\n");
	printf("PPR(x4)=%d gear=%d pulses/out-rev=%d sample=%dms duty=%u%%\r\n",
	       ENCODER_RESOLUTION, GEAR_RATIO, ENC_PULSES_PER_OUT_REV,
	       SAMPLE_PERIOD_MS, (unsigned)ENC_TEST_DUTY_PCT);
	printf("MotorA=left MotorB=right  FWD=CCW REV=CW (MOTOR_B_INVERT=%d)\r\n\r\n",
	       MOTOR_B_INVERT);

	ENC_MotorsStop();
	HAL_Delay(200);

	/* 1) Both wheels forward one revolution */
	printf("Phase1: both FWD 1 rev\r\n");
	if (ENC_DriveUntilPulses("P1-BOTH-FWD",
				 ENC_DIR_FWD, ENC_DIR_FWD,
				 1, 1, ENC_PULSES_PER_OUT_REV) != 0) {
		printf("Phase1 failed — check motor/encoder wiring.\r\n");
	}
	HAL_Delay(ENC_TEST_PHASE_GAP_MS);

	/* 2) Left forward, right backward, one revolution each */
	printf("Phase2: left FWD 1 rev, right REV 1 rev\r\n");
	if (ENC_DriveUntilPulses("P2-L-FWD-R-REV",
				 ENC_DIR_FWD, ENC_DIR_REV,
				 1, 1, ENC_PULSES_PER_OUT_REV) != 0) {
		printf("Phase2 failed — check motor/encoder wiring.\r\n");
	}
	HAL_Delay(ENC_TEST_PHASE_GAP_MS);

	/* 3) Both forward at fixed duty, loop encoder print */
	printf("Phase3: both FWD constant speed, print forever\r\n\r\n");
	printf("P3-CONST-FWD: duty=%u%%, printing every %ums (Ctrl+reset to stop)\r\n",
	       (unsigned)ENC_TEST_DUTY_PCT, (unsigned)ENC_TEST_PRINT_MS);
	ENC_ResetTotals();
	ENC_MotorsDrive(ENC_DIR_FWD, ENC_DIR_FWD, ENC_TEST_PWM);

	while (1) {
		ENC_PrintStatus("P3");
		HAL_Delay(ENC_TEST_PRINT_MS);
	}
}
