/*
 * usbd_dfu_qspi_flash.h
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

#ifndef __USBD_DFU_QSPI_FLASH_H__
#define __USBD_DFU_QSPI_FLASH_H__

#ifdef __cplusplus
 extern "C" {
#endif

#include "usbd_dfu.h"
#include "log.h"
#include "w25vq.h"

#define QSPI_FLASH_BASE		((uint32_t)0x90000000U)
#define QSPI_FLASH_END		((uint32_t)QSPI_FLASH_BASE + FLASH_BYTE)

#define DOWN_PROGRAM_BASE	((uint32_t)FLASH_BYTE / 2) //+2mb offset

#define QSPI_FLASH_DESC_STR      "@Winbond W25Q16 4MB   /0x90000000/32*4Kg"
// 8MB=128*(64KB blocks)=128*(16*4KB sectors)
#define QSPI_FLASH_ERASE_TIME    (uint16_t)20
#define QSPI_FLASH_PROGRAM_TIME  (uint16_t)20


extern USBD_DFU_MediaTypeDef USBD_DFU_QSPI_FLASH_fops_FS;

#ifdef __cplusplus
}
#endif

#endif /* __USBD_DFU_QSPI_FLASH_H__ */
