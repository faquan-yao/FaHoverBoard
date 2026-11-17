/*
 * Vofa.c
 *
 *  Created on: 2026年11月16日
 *      Author: yaofa
 */
#include "Vofa.h"

#include "stdio.h"
#include "string.h"
//#include "stm32f1xx_hal.h"
#include "usart.h"

// 改成你自己的串口句柄，比如 huart2
#define VOFA_UART_HANDLE    huart1

/**
 * @brief VOFA FireWater 文本协议发送
 * @param ch_num 通道数量
 * @param val float数据数组
 * @retval None
 * @note 格式 x0,x1,x2...\n，适合调试、通道少
 */
void Vofa_FireWater_Send(uint8_t ch_num, float *val)
{
    static char buf[128];
    uint16_t len = 0;
    for(uint8_t i = 0; i < ch_num; i++)
    {
        if(i == 0)
        {
            len += sprintf(buf + len, "%.3f", val[i]);
        }
        else
        {
            len += sprintf(buf + len, ",%.3f", val[i]);
        }
    }
    len += sprintf(buf + len, "\n");
    HAL_UART_Transmit(&VOFA_UART_HANDLE, (uint8_t *)buf, len, 10);
}

/**
 * @brief VOFA JustFloat 高速二进制协议发送(小端)
 * @param ch_num 通道数量
 * @param val float数据数组
 * @retval None
 * @note 帧尾固定 0x00 0x00 0x80 0x7F，PID/FOC高频采样首选
 */
void Vofa_JustFloat_Send(uint8_t ch_num, float *val)
{
    uint8_t tail[4] = {0x00, 0x00, 0x80, 0x7F};
    HAL_UART_Transmit(&VOFA_UART_HANDLE, (uint8_t *)val, ch_num * 4, 10);
    HAL_UART_Transmit(&VOFA_UART_HANDLE, tail, 4, 10);
}


/**
 *  @brief VOFA send IMU data
 *  @param pitch
 *  @param roll
 *  @param yaw
 *  @retval None
 */
void Vofa_Send_IMU(float pitch, float roll, float yaw)
{
	float imu_data[3] = {0};
	imu_data[0] = pitch;
	imu_data[1] = roll;
	imu_data[2] = yaw;

	Vofa_JustFloat_Send(3, imu_data);
}

void Vofa_Send_Filt_Data(float raw, float filt)
{
	float filt_data[2] = {0};
	filt_data[0] = raw;
	filt_data[1] = filt;

	Vofa_JustFloat_Send(2, filt_data);
}

/* ========= 使用示例 =========
float vofa_data[2] = {target, feedback};
Vofa_FireWater_Send(2, vofa_data);
Vofa_JustFloat_Send(2, vofa_data);
*/
