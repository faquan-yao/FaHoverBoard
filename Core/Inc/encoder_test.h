/*
 * encoder_test.h
 *
 *  Encoder board self-test (UART).
 */

#ifndef INC_ENCODER_TEST_H_
#define INC_ENCODER_TEST_H_

/**
 * Blocking encoder test loop: UART prints count, revolutions/direction, RPM.
 * Call after peripheral init + Enc_Init(). Does not return.
 */
void Enc_SelfTest(void);

#endif /* INC_ENCODER_TEST_H_ */
