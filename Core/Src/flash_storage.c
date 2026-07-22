/*
 * flash_storage.c
 *
 *  Last-page flash storage for calibration data (STM32F103xE, 2KB page).
 */
#include "flash_storage.h"
#include "stm32f1xx_hal.h"
#include <string.h>

uint32_t Flash_Crc32(const void *data, uint32_t len)
{
	const uint8_t *p = (const uint8_t *)data;
	uint32_t crc = 0xFFFFFFFFu;

	for (uint32_t i = 0; i < len; i++) {
		crc ^= p[i];
		for (int b = 0; b < 8; b++) {
			uint32_t mask = -(crc & 1u);
			crc = (crc >> 1) ^ (0xEDB88320u & mask);
		}
	}
	return ~crc;
}

int Flash_EraseCalPage(void)
{
	FLASH_EraseInitTypeDef erase = {0};
	uint32_t page_error = 0xFFFFFFFFu;
	HAL_StatusTypeDef st;

	HAL_FLASH_Unlock();
	erase.TypeErase = FLASH_TYPEERASE_PAGES;
	erase.PageAddress = FLASH_CAL_PAGE_ADDR;
	erase.NbPages = 1;
	st = HAL_FLASHEx_Erase(&erase, &page_error);
	HAL_FLASH_Lock();
	return (st == HAL_OK) ? 0 : -1;
}

int Flash_WriteCal(const void *data, uint32_t len)
{
	const uint8_t *src = (const uint8_t *)data;
	uint32_t addr = FLASH_CAL_PAGE_ADDR;
	uint8_t buf[256];
	uint32_t padded;
	HAL_StatusTypeDef st;

	if (data == NULL || len == 0U || len > FLASH_CAL_PAGE_SIZE) {
		return -1;
	}
	if (len > sizeof(buf)) {
		return -1;
	}

	memset(buf, 0xFF, sizeof(buf));
	memcpy(buf, src, len);
	padded = (len + 1U) & ~1U; /* halfword align */

	if (Flash_EraseCalPage() != 0) {
		return -1;
	}

	HAL_FLASH_Unlock();
	for (uint32_t i = 0; i < padded; i += 2U) {
		uint16_t half = (uint16_t)buf[i] | ((uint16_t)buf[i + 1U] << 8);
		st = HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, addr + i, half);
		if (st != HAL_OK) {
			HAL_FLASH_Lock();
			return -1;
		}
	}
	HAL_FLASH_Lock();
	return 0;
}

int Flash_ReadCal(void *data, uint32_t len)
{
	if (data == NULL || len == 0U || len > FLASH_CAL_PAGE_SIZE) {
		return -1;
	}
	memcpy(data, (const void *)FLASH_CAL_PAGE_ADDR, len);
	return 0;
}
