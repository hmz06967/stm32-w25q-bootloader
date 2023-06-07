/*
 * w25vq.h
 *
 *  Created on: May 25, 2023
 *      Author: hmz_o
 */

#ifndef INC_W25VQ_H_
#define INC_W25VQ_H_


#include "main.h"
#include <string.h>

#define FLASH_BYTE      ((4) * 1024 * 1024)  //4MB
#define SECTOR_BYTE     ((4) * 1024) //4kb
#define BLOCK_BYTE      ((64) * 1024) //64kb
#define PAGE_BYTE       ((1) * 256) //256byte

#define PAGE_SIZE       (FLASH_BYTE / PAGE_BYTE)//((1) * 1024) //1024*256 = 4mb
#define SECTOR_SIZE     (FLASH_BYTE / SECTOR_BYTE)
#define BLOCK_SIZE      (FLASH_BYTE / BLOCK_BYTE)

#define FLASH_OK        0
#define FLASH_ERR       1
#define FLASH_ERR_PARAM 2

uint8_t QSPI_Run(void);
uint8_t QSPI_Cmd(uint32_t cmd );

uint8_t QSPI_WriteEnable(void);
uint8_t QSPI_QEEnable(void);
uint8_t QSPI_QEDisable(void);
uint8_t QSPI_EnableMap(void);

uint8_t QSPI_IsBusy(void);
uint8_t QSPI_Reset(void) ;
uint8_t QSPI_Init(void);
void QSPI_Delay(uint32_t us);

uint8_t QSPI_ReadStatusReg(uint8_t *reg_data, uint8_t reg_num, uint8_t size);
uint8_t QSPI_WriteStatusReg(uint8_t *reg_data, uint8_t reg_num, uint8_t size);
uint16_t QSPI_ReadDevID(void);

uint8_t QSPI_EnableMemoryMappedMode(void);
uint8_t QSPI_AutoPollingMemReady(void);

uint8_t QSPI_EraseSector(uint32_t sector);
uint8_t QSPI_EraseBlock(uint32_t block);
uint8_t QSPI_EraseChip(void);
uint8_t QSPI_Write(uint32_t address, uint8_t *data, uint32_t size);
uint8_t QSPI_Read(uint32_t address, uint8_t *data, uint32_t size);
QSPI_CommandTypeDef QSPI_SetCom(uint32_t Instruction);


#endif /* INC_W25VQ_H_ */
