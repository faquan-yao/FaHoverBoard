/*
 * interrupt.h
 *
 *  Created on: Jan 18, 2026
 *      Author: yaofa
 */

#ifndef INC_INTERRUPT_H_
#define INC_INTERRUPT_H_
#include "tim.h"
#include "stm32f1xx_hal_tim.h"
#include "encoder.h"
#include "stdint.h"
#include "cmsis_os.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "stdio.h"

// 定义栈帧结构，匹配Cortex-M异常压栈顺序
typedef struct
{
  uint32_t R0;
  uint32_t R1;
  uint32_t R2;
  uint32_t R3;
  uint32_t R12;
  uint32_t LR;
  uint32_t PC;  // 异常发生前的PC值（关键）
  uint32_t xPSR;
} HardFault_StackFrame_t;

extern osMessageQueueId_t mEncoderQueueAHandle;
extern osMessageQueueId_t mEncoderQueueBHandle;

extern uint32_t ulHighFrequencyTimerTicks;

void HardFault(void);
void vPrintTaskList(void);
#endif /* INC_INTERRUPT_H_ */
