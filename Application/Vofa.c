/*
 * Vofa.c
 *
 *  Created on: 2026年11月16日
 *      Author: yaofa
 */
#include "Vofa.h"

#include "stdio.h"
#include "string.h"
#include "math.h"
#include "usart.h"
#include "pid_balance.h"

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

#define VOFA_CMD_MAX  40

static char vofa_cmd[VOFA_CMD_MAX];
static uint8_t vofa_cmd_len;

static int Vofa_Key(const char *s, const char *key, const char **rest)
{
	while (*key != '\0') {
		char c = *s++;
		if (c >= 'a' && c <= 'z') {
			c = (char)(c - 'a' + 'A');
		}
		if (c != *key++) {
			return 0;
		}
	}
	if (*s != '=') {
		return 0;
	}
	*rest = s + 1;
	return 1;
}

static int Vofa_ParseFloat(const char *s, float *out)
{
	int any = 0;
	int sign = 1;
	float v = 0.0f;
	float place = 0.1f;

	while (*s == ' ' || *s == '\t') {
		s++;
	}
	if (*s == '-') {
		sign = -1;
		s++;
	} else if (*s == '+') {
		s++;
	}
	while (*s >= '0' && *s <= '9') {
		v = v * 10.0f + (float)(*s - '0');
		s++;
		any = 1;
	}
	if (*s == '.') {
		s++;
		while (*s >= '0' && *s <= '9') {
			v += place * (float)(*s - '0');
			place *= 0.1f;
			s++;
			any = 1;
		}
	}
	if (!any) {
		return 0;
	}
	*out = (sign < 0) ? -v : v;
	return 1;
}

static void Vofa_ClampI(PID_TypeDef *pid)
{
	float lim = fabsf(pid->MaxI);
	pid->MaxI = lim;
	if (pid->Iout > lim) {
		pid->Iout = lim;
	}
	if (pid->Iout < -lim) {
		pid->Iout = -lim;
	}
}

static void Vofa_SetSymOut(PID_TypeDef *pid, float v)
{
	v = fabsf(v);
	pid->MaxOut = v;
	pid->MinOut = -v;
	if (pid->Out > pid->MaxOut) {
		pid->Out = pid->MaxOut;
	}
	if (pid->Out < pid->MinOut) {
		pid->Out = pid->MinOut;
	}
}

static void Vofa_PrintPid(void)
{
	printf("BK=%.3f BI=%.3f BD=%.3f BMAX=%.1f BILIM=%.2f "
	       "SK=%.3f SI=%.3f SD=%.3f SMAX=%.1f SILIM=%.2f ST=%.2f\r\n",
	       BalancePID.Kp, BalancePID.Ki, BalancePID.Kd,
	       BalancePID.MaxOut, BalancePID.MaxI,
	       SpeedPID.Kp, SpeedPID.Ki, SpeedPID.Kd,
	       SpeedPID.MaxOut, SpeedPID.MaxI,
	       speed_target_rpm);
}

static void Vofa_ApplyCmd(char *s)
{
	const char *num = NULL;
	float v;

	while (*s == ' ' || *s == '\t') {
		s++;
	}
	if (s[0] == '?' || strcmp(s, "GET") == 0) {
		Vofa_PrintPid();
		return;
	}

	if (Vofa_Key(s, "BILIM", &num) && Vofa_ParseFloat(num, &v)) {
		BalancePID.MaxI = v;
		Vofa_ClampI(&BalancePID);
	} else if (Vofa_Key(s, "BMAX", &num) && Vofa_ParseFloat(num, &v)) {
		Vofa_SetSymOut(&BalancePID, v);
	} else if (Vofa_Key(s, "SILIM", &num) && Vofa_ParseFloat(num, &v)) {
		SpeedPID.MaxI = v;
		Vofa_ClampI(&SpeedPID);
	} else if (Vofa_Key(s, "SMAX", &num) && Vofa_ParseFloat(num, &v)) {
		Vofa_SetSymOut(&SpeedPID, v);
	} else if (Vofa_Key(s, "BK", &num) && Vofa_ParseFloat(num, &v)) {
		BalancePID.Kp = v;
	} else if (Vofa_Key(s, "BI", &num) && Vofa_ParseFloat(num, &v)) {
		BalancePID.Ki = v;
		if (v == 0.0f) {
			BalancePID.Iout = 0.0f;
		}
	} else if (Vofa_Key(s, "BD", &num) && Vofa_ParseFloat(num, &v)) {
		BalancePID.Kd = v;
	} else if (Vofa_Key(s, "SK", &num) && Vofa_ParseFloat(num, &v)) {
		SpeedPID.Kp = v;
	} else if (Vofa_Key(s, "SI", &num) && Vofa_ParseFloat(num, &v)) {
		SpeedPID.Ki = v;
		if (v == 0.0f) {
			SpeedPID.Iout = 0.0f;
		}
	} else if (Vofa_Key(s, "SD", &num) && Vofa_ParseFloat(num, &v)) {
		SpeedPID.Kd = v;
	} else if (Vofa_Key(s, "ST", &num) && Vofa_ParseFloat(num, &v)) {
		speed_target_rpm = v;
	}
}

void Vofa_PidTune_Init(void)
{
	vofa_cmd_len = 0;
	UartRx_Start();
}

void Vofa_PidTune_Poll(void)
{
	uint8_t byte;

	while (UartRx_Read(&byte)) {
		if (byte == '!' || byte == '\n' || byte == '\r') {
			if (vofa_cmd_len > 0U) {
				vofa_cmd[vofa_cmd_len] = '\0';
				Vofa_ApplyCmd(vofa_cmd);
				vofa_cmd_len = 0;
			}
		} else if (vofa_cmd_len < (VOFA_CMD_MAX - 1U)) {
			vofa_cmd[vofa_cmd_len++] = (char)byte;
		} else {
			vofa_cmd_len = 0;
		}
	}
}
