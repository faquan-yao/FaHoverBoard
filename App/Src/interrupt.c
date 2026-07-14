/*
 * interrupt.c
 *
 *  Created on: Jan 18, 2026
 *      Author: yaofa
 */
#include "interrupt.h"

uint32_t ulHighFrequencyTimerTicks;

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
    uint32_t timeTick = HAL_GetTick();
    if (htim == &htim6) {
        EncoderData data;
        data.type = M;
        // TIM6溢出中断，读取TIM5编码器计数值并计算速度
        data.measuredValue = __HAL_TIM_GET_COUNTER(&htim5);
        data.timestamp = timeTick;
        xQueueOverwriteFromISR(mEncoderQueueAHandle, &data, 0);

        __HAL_TIM_SET_COUNTER(&htim5, 0);
        // 计算速度，假设编码器分辨率为1000线，轮子周长为1m
        //speed = (float)encoder_count * 4 / 1000.0 * 1.0 / 0.01;
    } else if (htim == &htim7) {
        EncoderData data;
        data.type = M;
        // TIM6溢出中断，读取TIM5编码器计数值并计算速度
        data.measuredValue = __HAL_TIM_GET_COUNTER(&htim3);
        data.timestamp = timeTick;
        xQueueOverwriteFromISR(mEncoderQueueBHandle, &data, 0);

        __HAL_TIM_SET_COUNTER(&htim3, 0);
        // 计算速度，假设编码器分辨率为1000线，轮子周长为1m
        //speed = (float)encoder_count * 4 / 1000.0 * 1.0 / 0.01;
    } else if (htim == &htim2) {
    	ulHighFrequencyTimerTicks++;
    }
}

void HardFault(void)
{
	  // 获取栈指针（MSP），转换为栈帧结构指针
	  HardFault_StackFrame_t *stackFrame = (HardFault_StackFrame_t *)__get_MSP();

	  // 打印故障信息和核心寄存器值（调试器Console或串口输出）
	  printf("=== HardFault Triggered ===\r\n");
	  printf("Fault PC: 0x%08X\r\n", stackFrame->PC);  // 触发错误的原始PC值
	  printf("Fault LR: 0x%08X\r\n", stackFrame->LR);
	  printf("CFSR: 0x%08X\r\n", SCB->CFSR);          // 故障状态寄存器
	  printf("BFAR: 0x%08X\r\n", SCB->BFAR);          // 总线故障地址

}

// 打印任务列表（需配合串口，函数名自定义）
void vPrintTaskList(void)
{
    printf("Task Name\tPriority\tStack Left\tTask State\r\n");
    // vTaskList：FreeRTOS官方函数，生成任务列表字符串
    char pcTaskList[512];
    vTaskList(pcTaskList);
    printf("%s\r\n", pcTaskList);
}
