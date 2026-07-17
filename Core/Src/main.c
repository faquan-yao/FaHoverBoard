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
#include "mpu6050.h"
#include "tb6612fng.h"
#include "encoder.h"
#include "pid_balance.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

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
int __io_putchar(int ch) {
	HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, 1000);
	return ch;
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
  /* MotorHwTest(); */ /* enable only when debugging motor wiring */

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

  Enc_Init();
  PID_Init();
  printf("Enter balance loop. ANGLE_MECH_ZERO=%.2f GYRO_BIAS=%.2f\r\n",
		 ANGLE_MECH_ZERO, GYRO_BIAS);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  float roll, yaw, pitch, gyro_dps;
  float angle_trim = 0.0f;
  float speed_filt = 0.0f;
  int16_t duty_filt = 0;
  uint32_t log_div = 0;
  uint32_t speed_div = 0;
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	  if (MPU6050_DMP_Get_Date(&pitch, &roll, &yaw, &gyro_dps) != 0) {
		  HAL_Delay(1);
		  continue;
	  }
	  gyro_dps -= GYRO_BIAS;

	  /* Speed loop ~10 Hz */
	  if ((++speed_div % 10U) == 0U) {
		  int32_t left, right;
		  ENC_GetSpeeds(&left, &right);
		  float spd = (float)(left + right) / 2.0f;
		  speed_filt = 0.8f * speed_filt + 0.2f * spd;
		  SpeedPID.NowVal = speed_filt;
		  SpeedPID.SetVal = 0.0f;
		  angle_trim = PID_Calc(&SpeedPID);
	  }

	  BalancePID.SetVal = ANGLE_MECH_ZERO + SPEED_TRIM_SIGN * angle_trim;
	  BalancePID.NowVal = pitch;
	  float pid_balance = PID_BalanceGyro(&BalancePID, gyro_dps);

	  int16_t duty = (int16_t)(12.0f * pid_balance);
	  if (duty > 900) duty = 900;
	  if (duty < -900) duty = -900;

	  {
		  const int16_t max_step = 35;
		  int16_t d = (int16_t)(duty - duty_filt);
		  if (d > max_step) d = max_step;
		  if (d < -max_step) d = -max_step;
		  duty_filt = (int16_t)(duty_filt + d);
	  }

	  MotorControl(duty_filt);

	  if ((++log_div % 50U) == 0U) {
		  printf("p=%.2f g=%.1f tgt=%.2f bal=%.2f duty=%d\r\n",
				 pitch, gyro_dps, BalancePID.SetVal, pid_balance, duty_filt);
	  }

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
