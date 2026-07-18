/*
 * encoder_test.c
 *
 *  Encoder self-test: pulse count, output-shaft revolutions/direction, RPM.
 */
#include "encoder_test.h"
#include "encoder.h"
#include "hardware_cfg.h"
#include "main.h"
#include <stdio.h>

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

void Enc_SelfTest(void)
{
	printf("\r\n==== Encoder self-test ====\r\n");
	printf("PPR(x4)=%d gear=%d pulses/out-rev=%d sample=%dms\r\n",
	       ENCODER_RESOLUTION, GEAR_RATIO, ENC_PULSES_PER_OUT_REV,
	       SAMPLE_PERIOD_MS);
	printf("Spin wheels by hand (or drive open-loop). Lines every 100ms.\r\n");
	printf("cnt=TIM raw  total=cum pulses  d=last 20ms pulses  rev=out-shaft\r\n\r\n");

	ENC_ResetTotals();

	while (1) {
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

		printf("L: cnt=%lu total=%ld d=%ld rev=%+.3f %s rpm=%+.1f | "
		       "R: cnt=%lu total=%ld d=%ld rev=%+.3f %s rpm=%+.1f\r\n",
		       (unsigned long)cnt_l, (long)total_l, (long)diff_l,
		       rev_l, ENC_DirStr(rpm_l), rpm_l,
		       (unsigned long)cnt_r, (long)total_r, (long)diff_r,
		       rev_r, ENC_DirStr(rpm_r), rpm_r);

		HAL_Delay(100);
	}
}
