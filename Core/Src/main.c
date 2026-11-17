/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "i2c.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdio.h"
#include "math.h"
#include "mpu6050.h"
#include "tb6612fng.h"
#include "encoder.h"
#include "encoder_test.h"
#include "mpu6050_test.h"
#include "accel_cal.h"
#include "pid_balance.h"
#include "Vofa.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* Enable only one test/cal mode at a time (blocks and skips balance loop). */
#ifndef RUN_ENCODER_TEST
#define RUN_ENCODER_TEST  0
#endif
#ifndef RUN_MPU_TEST
#define RUN_MPU_TEST      0
#endif
#ifndef RUN_ACCEL_CAL
#define RUN_ACCEL_CAL     0   /* 1 = MPU6050 six-face accel calibration */
#endif
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
static void Balance_CutAndReset(float *angle_trim, float *speed_filt, float *gyro_filt, int16_t *duty_filt)
{
	PID_Init();
	*angle_trim = 0.0f;
	*speed_filt = 0.0f;
	*gyro_filt = 0.0f;
	*duty_filt = 0;
	MotorShutdown();
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART1_UART_Init();
  MX_I2C2_Init();
  MX_TIM4_Init();
  MX_TIM1_Init();
  MX_TIM3_Init();
  MX_TIM5_Init();
  MX_TIM6_Init();
  /* USER CODE BEGIN 2 */
  MotorInit();
  MotorShutdown(); /* keep driver off until balance loop recovers upright */
  /* MotorHwTest(); */ /* enable only when debugging motor wiring */

  Enc_Init();
#if RUN_ENCODER_TEST
  Enc_SelfTest(); /* UART count / rev / RPM — does not return */
#endif

  HAL_Delay(100); /* MPU6050 power-up settle */

  /* Probe I2C: AD0=GND → 0xD0, AD0=VCC → 0xD2 (HAL 8-bit addr) */
  if (HAL_I2C_IsDeviceReady(&hi2c2, 0xD0, 5, 200) == HAL_OK) {
	  printf("MPU6050 ACK @ 0x68 (0xD0)\r\n");
  } else if (HAL_I2C_IsDeviceReady(&hi2c2, 0xD2, 5, 200) == HAL_OK) {
	  printf("MPU6050 ACK @ 0x69 (0xD2) — check AD0 / addr in inv_mpu.c\r\n");
  } else {
	  printf("MPU6050 no ACK on I2C2 (PB10/SCL, PB11/SDA). Check wiring/power.\r\n");
  }

  int ret = 0;
  do {
	  ret = MPU6050_DMP_init();
	  if (ret != 0) {
		  printf("MPU6050 init failure, ret = %d!\r\n", ret);
		  HAL_Delay(500);
	  }
  } while (ret);
  printf("MPU6050 Ready!\r\n");

#if RUN_MPU_TEST
  MPU_SelfTest(); /* UART: pitch, roll, yaw, gyro_dps — does not return */
#endif
#if RUN_ACCEL_CAL
  AccelCal_SelfTest(); /* six-face cal → flash — does not return */
#endif

  PID_Init();
  MotorShutdown();
  Vofa_PidTune_Init();
  printf("Enter balance loop. ANGLE_MECH_ZERO=%.2f GYRO_BIAS=%.2f FALL=%.0f\r\n",
		 ANGLE_MECH_ZERO, GYRO_BIAS, FALL_ANGLE_DEG);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  float roll, yaw, pitch, gyro_dps;
  float angle_trim = 0.0f;
  float speed_filt = 0.0f;
  float gyro_filt = 0.0f;
  int16_t duty_filt = 0;
  uint32_t log_div = 0;
  uint32_t speed_div = 0;
  uint8_t fallen = 1; /* start cut off until upright and settled */
  uint32_t last_imu_ms = HAL_GetTick();
  uint32_t recover_ok_ms = 0;
  (void)speed_filt;  /* used when speed loop is enabled */
  (void)speed_div;
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	  Vofa_PidTune_Poll();
	  if (MPU6050_DMP_Get_Date(&pitch, &roll, &yaw, &gyro_dps) != 0) {
		  if ((HAL_GetTick() - last_imu_ms) > IMU_FAIL_KILL_MS) {
			  if (!fallen) {
				  fallen = 1;
				  recover_ok_ms = 0;
				  printf("IMU timeout, motors off\r\n");
			  }
			  Balance_CutAndReset(&angle_trim, &speed_filt, &gyro_filt, &duty_filt);
		  }
		  HAL_Delay(1);
		  continue;
	  }
	  last_imu_ms = HAL_GetTick();
//	  Vofa_Send_IMU(pitch, roll, yaw);
	  gyro_dps -= GYRO_BIAS;
	  gyro_filt = (1.0f - GYRO_LPF_ALPHA) * gyro_filt + GYRO_LPF_ALPHA * gyro_dps;
	  {
		  float ang_err = fabsf(pitch - ANGLE_MECH_ZERO);
		  if (!fallen) {
			  if (ang_err > FALL_ANGLE_DEG) {
				  fallen = 1;
				  recover_ok_ms = 0;
				  Balance_CutAndReset(&angle_trim, &speed_filt, &gyro_filt, &duty_filt);
				  printf("Fallen, motors off pitch=%.2f\r\n", pitch);
			  }
		  } else if (ang_err < RECOVER_ANGLE_DEG && fabsf(gyro_dps) < RECOVER_GYRO_DPS) {
			  if (recover_ok_ms == 0U) {
				  recover_ok_ms = HAL_GetTick();
			  }
			  if ((HAL_GetTick() - recover_ok_ms) >= RECOVER_HOLD_MS) {
				  fallen = 0;
				  recover_ok_ms = 0;
				  PID_Init();
				  angle_trim = 0.0f;
				  speed_filt = 0.0f;
				  duty_filt = 0;
				  printf("Recovered, motors on pitch=%.2f\r\n", pitch);
			  }
		  } else {
			  recover_ok_ms = 0;
		  }
	  }

	  if (fallen) {
		  MotorShutdown();
		  if ((++log_div % 10U) == 0U) {
			  printf("%.2f,%.2f,0,0\r\n", pitch, ANGLE_MECH_ZERO);
		  }
		  HAL_GPIO_TogglePin(PULSE_LIGHT_GPIO_Port, PULSE_LIGHT_Pin);
		  continue;
	  }

	  /* Speed loop ~10 Hz, feedback in output-shaft RPM */
	  if ((++speed_div % 10U) == 0U) {
		  float left_rpm, right_rpm;
		  ENC_GetSpeeds(&left_rpm, &right_rpm);
		  //两个轮子由于是对向安装的，他们的速度方向是相反的，求和应该是（left_rpm + (-right_rpm)）/ 2.0,化简之后如下
		  float spd = (left_rpm - right_rpm) / 2.0f;
		  speed_filt = 0.8f * speed_filt + 0.2f * spd;
//		  Vofa_Send_Filt_Data(spd, speed_filt);
		  SpeedPID.NowVal = speed_filt;
		  SpeedPID.SetVal = speed_target_rpm;
		  angle_trim = PID_Calc(&SpeedPID);
	  }
	  BalancePID.SetVal = ANGLE_MECH_ZERO + SPEED_TRIM_SIGN * angle_trim;
	  BalancePID.NowVal = pitch;
	  float pid_balance = PID_BalanceGyro(&BalancePID, /*gyro_filt*/0);

	  float vofa_pid_data[] = {BalancePID.SetVal, BalancePID.NowVal, gyro_filt, pid_balance};
	  Vofa_JustFloat_Send(4, vofa_pid_data);

	  int16_t duty = (int16_t)(MOTOR_CMD_SIGN * 9.0f * pid_balance);

	  duty_filt = 0.8f * duty_filt + 0.2f * duty;

	  float duty_output = duty_filt;
	  if (duty_output > 900) duty_output = 900;
	  if (duty_output < -900) duty_output = -900;

	  MotorControl(duty_output);

//	  float vofa_data[3] = {pitch, speed_filt, duty_output};
//	  Vofa_JustFloat_Send(3, vofa_data);

	  HAL_GPIO_TogglePin(PULSE_LIGHT_GPIO_Port, PULSE_LIGHT_Pin);
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
