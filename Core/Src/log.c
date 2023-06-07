/*
 * log.c
 *
 *  Created on: May 13, 2023
 *      Author: hmz_o
 */

#include "log.h"
#ifdef LOG
#define FRIMWARE_NAME_LEN_MAX               64U
#define REGISTER_CODE_LEN_MAX               (4*4U)

extern UART_HandleTypeDef huart5;
char pData[255];

int _write_(int file, char *ptr, int len) {
  HAL_UART_Transmit(&huart5, (uint8_t*)ptr, len, HAL_MAX_DELAY);
  return len;
}

int __io_putchar(int ch)
{
  uint8_t data = (uint8_t)ch;
  HAL_UART_Transmit(&huart5, &data, 1, HAL_MAX_DELAY);
  return ch;
}

#endif
