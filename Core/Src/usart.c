/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    usart.c
  * @brief   This file provides code for the configuration
  *          of the USART instances.
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
#include "usart.h"

/* USER CODE BEGIN 0 */
#include <string.h>

/* TX ring: printf writes here, USART1 TXE IRQ drains it.
 * Keep one slot empty so w==r means empty. Never drop bytes. */
#ifndef UART_TX_BUF_SIZE
#define UART_TX_BUF_SIZE  4096U
#endif

static uint8_t uart_tx_buf[UART_TX_BUF_SIZE];
static volatile uint16_t uart_tx_w;
static volatile uint16_t uart_tx_r;

static uint16_t UartTx_Free(void)
{
	uint16_t r = uart_tx_r;
	uint16_t w = uart_tx_w;
	if (w >= r) {
		return (uint16_t)(UART_TX_BUF_SIZE - 1U - (w - r));
	}
	return (uint16_t)(r - w - 1U);
}

static void UartTx_Kick(void)
{
	__HAL_UART_ENABLE_IT(&huart1, UART_IT_TXE);
}

static void UartTx_Put(const uint8_t *data, uint16_t len)
{
	while (len > 0U) {
		uint16_t free;
		do {
			free = UartTx_Free();
		} while (free == 0U);

		uint16_t w = uart_tx_w;
		uint16_t chunk = free;
		if (chunk > len) {
			chunk = len;
		}
		uint16_t to_end = (uint16_t)(UART_TX_BUF_SIZE - w);
		if (chunk > to_end) {
			chunk = to_end;
		}
		memcpy(&uart_tx_buf[w], data, chunk);
		w = (uint16_t)(w + chunk);
		if (w >= UART_TX_BUF_SIZE) {
			w = 0U;
		}
		uart_tx_w = w;
		data += chunk;
		len = (uint16_t)(len - chunk);
		UartTx_Kick();
	}
}

void UartTx_IRQHandler(void)
{
	if (__HAL_UART_GET_FLAG(&huart1, UART_FLAG_TXE) &&
	    __HAL_UART_GET_IT_SOURCE(&huart1, UART_IT_TXE)) {
		if (uart_tx_r == uart_tx_w) {
			__HAL_UART_DISABLE_IT(&huart1, UART_IT_TXE);
		} else {
			huart1.Instance->DR = uart_tx_buf[uart_tx_r];
			uint16_t r = (uint16_t)(uart_tx_r + 1U);
			if (r >= UART_TX_BUF_SIZE) {
				r = 0U;
			}
			uart_tx_r = r;
		}
	}
	if (__HAL_UART_GET_FLAG(&huart1, UART_FLAG_ORE)) {
		__HAL_UART_CLEAR_OREFLAG(&huart1);
	}
}

int __io_putchar(int ch)
{
	uint8_t c = (uint8_t)ch;
	UartTx_Put(&c, 1U);
	return ch;
}

int _write(int file, char *ptr, int len)
{
	(void)file;
	int remain = len;
	while (remain > 0) {
		uint16_t chunk = (remain > 512) ? 512U : (uint16_t)remain;
		UartTx_Put((const uint8_t *)ptr, chunk);
		ptr += chunk;
		remain -= (int)chunk;
	}
	return len;
}
/* USER CODE END 0 */

UART_HandleTypeDef huart1;

/* USART1 init function */

void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

void HAL_UART_MspInit(UART_HandleTypeDef* uartHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(uartHandle->Instance==USART1)
  {
  /* USER CODE BEGIN USART1_MspInit 0 */

  /* USER CODE END USART1_MspInit 0 */
    /* USART1 clock enable */
    __HAL_RCC_USART1_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**USART1 GPIO Configuration
    PA9     ------> USART1_TX
    PA10     ------> USART1_RX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* USER CODE BEGIN USART1_MspInit 1 */
    HAL_NVIC_SetPriority(USART1_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(USART1_IRQn);
  /* USER CODE END USART1_MspInit 1 */
  }
}

void HAL_UART_MspDeInit(UART_HandleTypeDef* uartHandle)
{

  if(uartHandle->Instance==USART1)
  {
  /* USER CODE BEGIN USART1_MspDeInit 0 */

  /* USER CODE END USART1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_USART1_CLK_DISABLE();

    /**USART1 GPIO Configuration
    PA9     ------> USART1_TX
    PA10     ------> USART1_RX
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_9|GPIO_PIN_10);

  /* USER CODE BEGIN USART1_MspDeInit 1 */
    HAL_NVIC_DisableIRQ(USART1_IRQn);
  /* USER CODE END USART1_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
