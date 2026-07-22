/*
 * encoder_test.h
 *
 *  Encoder board self-test (UART + open-loop motors).
 */

#ifndef INC_ENCODER_TEST_H_
#define INC_ENCODER_TEST_H_

/**
 * Blocking encoder self-test (does not return):
 *  1) Both wheels forward 1 output-shaft revolution
 *  2) Left forward 1 rev, right backward 1 rev
 *  3) Both forward at fixed duty, UART print encoder forever
 *
 * Default PWM duty = 25%. Call after MotorInit() + Enc_Init().
 */
void Enc_SelfTest(void);

#endif /* INC_ENCODER_TEST_H_ */
