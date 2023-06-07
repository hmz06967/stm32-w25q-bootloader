/*
 * log.h
 *
 *  Created on: May 16, 2023
 *      Author: hmz_o
 */

#ifndef INC_LOG_H_
#define INC_LOG_H_

#include "main.h"
#include <stdio.h>
#include <string.h>
#include <stdio.h>


#define FS_WRITE 0x01

void log_fs_console(uint8_t *src, uint8_t *dest, uint32_t Len, uint8_t cmd);

#endif /* INC_LOG_H_ */
