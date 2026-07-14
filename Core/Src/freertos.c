/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "encoder.h"
#include "stdio.h"
#include "queue.h"
#include "stdlib.h"
#include "mpu6050.h"
#include "pid.h"
#include "tim.h"
#include "interrupt.h"
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
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
/* Definitions for mainTask */
osThreadId_t mainTaskHandle;
const osThreadAttr_t mainTask_attributes = {
  .name = "mainTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for speedTask */
osThreadId_t speedTaskHandle;
const osThreadAttr_t speedTask_attributes = {
  .name = "speedTask",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for mpuTask */
osThreadId_t mpuTaskHandle;
const osThreadAttr_t mpuTask_attributes = {
  .name = "mpuTask",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for attitudeTask */
osThreadId_t attitudeTaskHandle;
const osThreadAttr_t attitudeTask_attributes = {
  .name = "attitudeTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for ctrlTask */
osThreadId_t ctrlTaskHandle;
const osThreadAttr_t ctrlTask_attributes = {
  .name = "ctrlTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for uartTask */
osThreadId_t uartTaskHandle;
const osThreadAttr_t uartTask_attributes = {
  .name = "uartTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for mEncoderQueueA */
osMessageQueueId_t mEncoderQueueAHandle;
const osMessageQueueAttr_t mEncoderQueueA_attributes = {
  .name = "mEncoderQueueA"
};
/* Definitions for mEncoderQueueB */
osMessageQueueId_t mEncoderQueueBHandle;
const osMessageQueueAttr_t mEncoderQueueB_attributes = {
  .name = "mEncoderQueueB"
};
/* Definitions for mSpeedCtrlQueue */
osMessageQueueId_t mSpeedCtrlQueueHandle;
const osMessageQueueAttr_t mSpeedCtrlQueue_attributes = {
  .name = "mSpeedCtrlQueue"
};
/* Definitions for mSteerQueue */
osMessageQueueId_t mSteerQueueHandle;
const osMessageQueueAttr_t mSteerQueue_attributes = {
  .name = "mSteerQueue"
};
/* Definitions for mAttitudeCtrlQueue1 */
osMessageQueueId_t mAttitudeCtrlQueue1Handle;
const osMessageQueueAttr_t mAttitudeCtrlQueue1_attributes = {
  .name = "mAttitudeCtrlQueue1"
};
/* Definitions for myMpuDataQueue */
osMessageQueueId_t myMpuDataQueueHandle;
const osMessageQueueAttr_t myMpuDataQueue_attributes = {
  .name = "myMpuDataQueue"
};
/* Definitions for mSpeedCalibrateQueue */
osMessageQueueId_t mSpeedCalibrateQueueHandle;
const osMessageQueueAttr_t mSpeedCalibrateQueue_attributes = {
  .name = "mSpeedCalibrateQueue"
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartMainTask(void *argument);
void StartSpeedTask(void *argument);
void StartMpuTask(void *argument);
void StartAttitudeTask(void *argument);
void StartCtrlTask(void *argument);
void StartuartTask(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* Hook prototypes */
void configureTimerForRunTimeStats(void);
unsigned long getRunTimeCounterValue(void);

/* USER CODE BEGIN 1 */
/* Functions needed when configGENERATE_RUN_TIME_STATS is on */
__weak void configureTimerForRunTimeStats(void)
{
	HAL_TIM_Base_Start_IT(&htim2);
}

__weak unsigned long getRunTimeCounterValue(void)
{
	return ulHighFrequencyTimerTicks;
}
/* USER CODE END 1 */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the queue(s) */
  /* creation of mEncoderQueueA */
  mEncoderQueueAHandle = osMessageQueueNew (1, sizeof(uint16_t), &mEncoderQueueA_attributes);

  /* creation of mEncoderQueueB */
  mEncoderQueueBHandle = osMessageQueueNew (1, sizeof(uint16_t), &mEncoderQueueB_attributes);

  /* creation of mSpeedCtrlQueue */
  mSpeedCtrlQueueHandle = osMessageQueueNew (1, sizeof(uint16_t), &mSpeedCtrlQueue_attributes);

  /* creation of mSteerQueue */
  mSteerQueueHandle = osMessageQueueNew (1, sizeof(uint16_t), &mSteerQueue_attributes);

  /* creation of mAttitudeCtrlQueue1 */
  mAttitudeCtrlQueue1Handle = osMessageQueueNew (1, sizeof(uint16_t), &mAttitudeCtrlQueue1_attributes);

  /* creation of myMpuDataQueue */
  myMpuDataQueueHandle = osMessageQueueNew (1, sizeof(int32_t), &myMpuDataQueue_attributes);

  /* creation of mSpeedCalibrateQueue */
  mSpeedCalibrateQueueHandle = osMessageQueueNew (1, sizeof(uint16_t), &mSpeedCalibrateQueue_attributes);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of mainTask */
  mainTaskHandle = osThreadNew(StartMainTask, NULL, &mainTask_attributes);

  /* creation of speedTask */
  speedTaskHandle = osThreadNew(StartSpeedTask, NULL, &speedTask_attributes);

  /* creation of mpuTask */
  mpuTaskHandle = osThreadNew(StartMpuTask, NULL, &mpuTask_attributes);

  /* creation of attitudeTask */
  attitudeTaskHandle = osThreadNew(StartAttitudeTask, NULL, &attitudeTask_attributes);

  /* creation of ctrlTask */
  ctrlTaskHandle = osThreadNew(StartCtrlTask, NULL, &ctrlTask_attributes);

  /* creation of uartTask */
  uartTaskHandle = osThreadNew(StartuartTask, NULL, &uartTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  if (mainTaskHandle == NULL) {
	  printf("Thread main created failed.\r\n");
	  while(1);
  }
  if (speedTaskHandle == NULL) {
	  printf("Thread speed created failed.\r\n");
	  while(1);
  }
  if (mpuTaskHandle == NULL) {
	  printf("Thread mpu created failed.\r\n");
	  while(1);
  }
  if (attitudeTaskHandle == NULL) {
	  printf("Thread attitude created failed.\r\n");
	  while(1);
  }
  if (ctrlTaskHandle == NULL) {
	  printf("Thread ctrl created failed.\r\n");
	  while(1);
  }
  if (uartTaskHandle == NULL) {
	  printf("Thread uart created failed.\r\n");
	  while(1);
  }
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartMainTask */
/**
  * @brief  Function implementing the mainTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartMainTask */
void StartMainTask(void *argument)
{
  /* USER CODE BEGIN StartMainTask */
	printf("start main task.....\r\n");
  /* Infinite loop */
//  Enc_init();
  for(;;)
  {
	  HAL_GPIO_TogglePin(PULSE_LIGHT_GPIO_Port, PULSE_LIGHT_Pin);
//	  uint32_t timeTick = HAL_GetTick();
//	  printf("now: %ld.\r\n", timeTick);
	  uint32_t free_heap = xPortGetFreeHeapSize();
	  printf("total free heap: %ld.\r\n", free_heap);
	  osDelay(1000);
  }
  /* USER CODE END StartMainTask */
}

/* USER CODE BEGIN Header_StartSpeedTask */
/**
* @brief Function implementing the speedTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartSpeedTask */
void StartSpeedTask(void *argument)
{
  /* USER CODE BEGIN StartSpeedTask */
	printf("start speed task.....\r\n");
  /* Infinite loop */
  EncoderData encoderA;
  EncoderData encoderB;
  int16_t targetSpeed = 0;
  float speedA = 0;
  float speedB = 0;
  BaseType_t xStatus;

  PID_HandleTypeDef wheelA, wheelB;
  PID_Init(&wheelA, 2.0f, 0.1f, 0.5f, 3.0f, -3.0f, 0.5f);
  PID_Init(&wheelB, 2.0f, 0.1f, 0.5f, 3.0f, -3.0f, 0.5f);

  for(;;)
  {
    xStatus = xQueuePeek(mEncoderQueueAHandle, &encoderA, 0);
    if (xStatus == pdPASS){
    	//float speed = (60 * encoderA.measuredValue) / (2 * 0.02 * 500 * 28);
    	speedA = encoderA.measuredValue * 0.11;
//        printf("wheel A speed : %f.\r\n", speedA);
    }
    xStatus = xQueuePeek(mEncoderQueueBHandle, &encoderB, 0);
    if (xStatus == pdPASS) {
        //float speed = (60 * encoderB.measuredValue) / (2 * 0.02 * 500 * 28);
    	speedB = encoderB.measuredValue * 0.11;
//    	printf("wheel B speed : %f\r\n", speedB);
    }
    xStatus = xQueuePeek(mSpeedCtrlQueueHandle, &targetSpeed, 0);
    xStatus = pdPASS;
    if (xStatus == pdPASS) {
    	printf("tartSpeed %d.\r\n", targetSpeed);
       float pidValueA = PID_Calculate(&wheelA, targetSpeed, speedA);
       float pidValueB = PID_Calculate(&wheelB, targetSpeed, speedB);
       printf("pid: %f\r\n", pidValueA);
    }
    uint32_t task_stack_free = uxTaskGetStackHighWaterMark(NULL) * 4;
    printf("Task Speed free stack: %ld.\r\n", task_stack_free);
    osDelay(200);
  }
  /* USER CODE END StartSpeedTask */
}

/* USER CODE BEGIN Header_StartMpuTask */
/**
* @brief Function implementing the mpuTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartMpuTask */
void StartMpuTask(void *argument)
{
  /* USER CODE BEGIN StartMpuTask */
	printf("start mpu task.....\r\n");
  /* Infinite loop */
  int ret = 0;
  do {
	  ret = MPU6050_DMP_init();
	  if(ret <= 0) {
		  printf("MPU6050 init failure, ret = %d!\r\n", ret);
	  }
  } while(ret);
  printf("MPU6050 Ready!\r\n");
  int32_t roll, yaw, pitch;
  for(;;)
  {
	int32_t pitch_cdeg;
	int32_t roll_cdeg;
	int32_t yaw_cdeg;

	printf("%ld: read mpu6050.\r\n", HAL_GetTick());
    MPU6050_DMP_Get_Date(&pitch, &roll, &yaw);
    xQueueOverwrite(myMpuDataQueueHandle, &pitch);
    pitch_cdeg = q30_to_centi_deg(pitch);
    roll_cdeg = q30_to_centi_deg(roll);
    yaw_cdeg = q30_to_centi_deg(yaw);
    printf("pitch: %ld.%02ld deg, roll: %ld.%02ld deg, yaw: %ld.%02ld deg\r\n",
           pitch_cdeg / 100, (pitch_cdeg < 0 ? -pitch_cdeg : pitch_cdeg) % 100,
           roll_cdeg / 100, (roll_cdeg < 0 ? -roll_cdeg : roll_cdeg) % 100,
           yaw_cdeg / 100, (yaw_cdeg < 0 ? -yaw_cdeg : yaw_cdeg) % 100);
    printf("%ld: read mpu6050 end.\r\n", HAL_GetTick());

    uint32_t task_stack_free = uxTaskGetStackHighWaterMark(NULL) * 4;
    printf("Task mpu free stack: %ld.\r\n", task_stack_free);

    osDelay(100);
  }
  /* USER CODE END StartMpuTask */
}

/* USER CODE BEGIN Header_StartAttitudeTask */
/**
* @brief Function implementing the attitudeTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartAttitudeTask */
void StartAttitudeTask(void *argument)
{
  /* USER CODE BEGIN StartAttitudeTask */
	printf("start attitude task.....\r\n");
  /* Infinite loop */
  for(;;)
  {
    osDelay(1000);
  }
  /* USER CODE END StartAttitudeTask */
}

/* USER CODE BEGIN Header_StartCtrlTask */
/**
* @brief Function implementing the ctrlTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartCtrlTask */
void StartCtrlTask(void *argument)
{
  /* USER CODE BEGIN StartCtrlTask */
	printf("start ctrl task.....\r\n");
  /* Infinite loop */
  for(;;)
  {
    osDelay(1000);
  }
  /* USER CODE END StartCtrlTask */
}

/* USER CODE BEGIN Header_StartuartTask */
/**
* @brief Function implementing the uartTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartuartTask */
void StartuartTask(void *argument)
{
  /* USER CODE BEGIN StartuartTask */
  /* Infinite loop */
  printf("start uart task.....\r\n");
	int16_t speed = 0;
	int16_t steer = 0;
  for(;;)
  {
	xQueueOverwrite(mSpeedCtrlQueueHandle, &speed);
	xQueueOverwrite(mSteerQueueHandle, &steer);
	osDelay(100);
  }
  /* USER CODE END StartuartTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

