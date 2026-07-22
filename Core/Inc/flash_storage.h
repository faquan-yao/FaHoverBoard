/*
 * flash_storage.h
 *
 *  Persist small records in the last STM32F103xE flash page.
 */
#ifndef INC_FLASH_STORAGE_H_
#define INC_FLASH_STORAGE_H_

#include <stdint.h>

/* STM32F103ZE: 512KB flash, 2KB pages — last page */
#ifndef FLASH_CAL_PAGE_ADDR
#define FLASH_CAL_PAGE_ADDR  0x0807F800UL
#endif
#define FLASH_CAL_PAGE_SIZE  0x800U

int Flash_EraseCalPage(void);
int Flash_WriteCal(const void *data, uint32_t len);
int Flash_ReadCal(void *data, uint32_t len);
uint32_t Flash_Crc32(const void *data, uint32_t len);

#endif /* INC_FLASH_STORAGE_H_ */
