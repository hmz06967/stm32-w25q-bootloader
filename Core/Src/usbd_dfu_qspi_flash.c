/*
 * usbd_dfu_qspi_flash.c
 *
 * QSPI flash interface for DFU media access.
 *
 *  Created on: Jan 21, 2020
 *      Author: affe00
------------------------------------------------------------------------------
 BSD 3-Clause License

Copyright (c) 2020, affe00
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its
   contributors may be used to endorse or promote products derived from
   this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
------------------------------------------------------------------------------
 */
#include "usbd_dfu_qspi_flash.h"

extern USBD_HandleTypeDef hUsbDeviceFS;
extern QSPI_HandleTypeDef hqspi;

static uint16_t QSPI_If_Init_FS(void);
static uint16_t QSPI_If_Erase_FS(uint32_t Add);
static uint16_t QSPI_If_Write_FS(uint8_t *src, uint8_t *dest, uint32_t Len);
static uint8_t *QSPI_If_Read_FS(uint8_t *src, uint8_t *dest, uint32_t Len);
static uint16_t QSPI_If_DeInit_FS(void);
static uint16_t QSPI_If_GetStatus_FS(uint32_t Add, uint8_t Cmd, uint8_t *buffer);

uint16_t state = USBD_OK;

/**
  * @}
  */

#if defined ( __ICCARM__ ) /* IAR Compiler */
  #pragma data_alignment=4
#endif
__ALIGN_BEGIN USBD_DFU_MediaTypeDef USBD_DFU_QSPI_FLASH_fops_FS __ALIGN_END =
{
   (uint8_t*)QSPI_FLASH_DESC_STR,
   QSPI_If_Init_FS,
   QSPI_If_DeInit_FS,
   QSPI_If_Erase_FS,
   QSPI_If_Write_FS,
   QSPI_If_Read_FS,
   QSPI_If_GetStatus_FS
};

/**
  * @brief  Memory initialization routine.
  * @retval USBD_OK if operation is successful,  else.
  */
uint16_t QSPI_If_Init_FS(void){

	return (USBD_OK);
}

/**
  * @brief  De-Initializes Memory
  * @retval USBD_OK if operation is successful,  else
  */
uint16_t QSPI_If_DeInit_FS(void)
{
  return (USBD_OK);
}

/**
  * @brief  Erase block(64KB).
  * @param  Add: Address of sector to be erased.
  * @retval 0 if operation is successful,  else.
  */
uint16_t QSPI_If_Erase_FS(uint32_t Add){
	//printf("erase flag partition ...\r\n");
	state = USBD_FAIL;
	///blue led on

	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, GPIO_PIN_SET);
	printf("erase flag partition..\r\n");

	if (Add <= QSPI_FLASH_END){
		state = QSPI_EraseBlock(Add);
		state = HAL_OK;
	}
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, GPIO_PIN_RESET);

	return state;
}

/**
  * @brief  Memory write routine.
  * @param  src: Pointer to the source buffer. Address to be written to.
  * @param  dest: Pointer to the destination buffer.
  * @param  Len: Number of data to be written (in bytes).
  * @retval USBD_OK if operation is successful,  else.
  */

uint32_t back_addr = 0;

void erase_sector(uint32_t addr){
	uint32_t block_er = 0;
	if(((addr - back_addr) % SECTOR_BYTE  == 0)){
		block_er = addr / SECTOR_BYTE;
		QSPI_EraseSector(block_er);
		back_addr = addr;
		printf("er(%ld) ", block_er);
	}
}

uint16_t QSPI_If_Write_FS(uint8_t *src, uint8_t *dest, uint32_t Len)
{
	state = USBD_FAIL;
	///blue led on
	//HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, GPIO_PIN_SET);
	//HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);

	printf("writing: ");
	uint32_t addr = ((uint32_t)dest-QSPI_FLASH_BASE);//flasha yazma adresi (0 + down_addr)

	erase_sector(addr);

	if(dest < (uint8_t*)QSPI_FLASH_BASE){
		addr = ((uint32_t)dest-FLASH_BANK1_BASE);
	}
	if ((addr+Len)<=FLASH_BYTE){
		state =  QSPI_Write(addr,src, Len);
	}

	printf(" 0x%lX, %ld OK\r\n", addr, Len);

	//HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, GPIO_PIN_RESET);
	//HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);
	HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_0);
	return state;
}

/**
  * @brief  Memory read routine.
  * @param  src: Pointer to the source buffer. Address to be written to.
  * @param  dest: Pointer to the destination buffer.
  * @param  Len: Number of data to be read (in bytes).
  * @retval Pointer to the physical address where data should be read.
  */
uint8_t *QSPI_If_Read_FS(uint8_t *src, uint8_t *dest, uint32_t Len){
	if(DEBUG){
		printf("Read addr: 0x%lX, 0x%lX, %ld \r\n", (uint32_t)src, (uint32_t)dest, (uint32_t)Len);
	}
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, GPIO_PIN_SET);
	if (src>=(uint8_t*)QSPI_FLASH_BASE && (src+Len)<=(uint8_t*)QSPI_FLASH_END){
		QSPI_Read(((uint32_t)src-QSPI_FLASH_BASE), dest, Len);
	}
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, GPIO_PIN_RESET);
	return (uint8_t *) (dest);
	/* no way to indicate fail from return value */
}

/**
  * @brief  Get status routine
  * @param  Add: Address to be read from
  * @param  Cmd: Number of data to be read (in bytes)
  * @param  buffer: used for returning the time necessary for a program or an erase operation
  * @retval USBD_OK if operation is successful
  */
uint16_t QSPI_If_GetStatus_FS(uint32_t Add, uint8_t Cmd, uint8_t *buffer)
{
  switch (Cmd)
  {
    case DFU_MEDIA_PROGRAM:
	buffer[1] = (uint8_t) QSPI_FLASH_PROGRAM_TIME;
	buffer[2] = (uint8_t) (QSPI_FLASH_PROGRAM_TIME << 8);
	buffer[3] = 0;
    break;

    case DFU_MEDIA_ERASE:
    default:
	buffer[1] = (uint8_t) QSPI_FLASH_ERASE_TIME;
	buffer[2] = (uint8_t) (QSPI_FLASH_ERASE_TIME << 8);
	buffer[3] = 0;
	QSPI_EraseChip();
    break;
  }
  return (USBD_OK);
}

