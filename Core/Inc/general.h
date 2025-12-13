/*
 * general.h
 *
 *  Created on: Nov 14, 2025
 *      Author: dobao
 */

#ifndef INC_GENERAL_H_
#define INC_GENERAL_H_

#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>

#include "stm32f405_addr.h"
#include "flash.h"
#include "rcc.h"

typedef enum{
	PERIPH_GPIO,
	PERIPH_FLASH,
	PERIPH_SPI,
	PERIPH_I2C,
	PERIPH_UART,
	PERIPH_RCC
}Peripheral_t;

bool sanityCheckField(uint8_t bitPosition, uint8_t bitWidth);
bool sanityCheckFieldValue(uint8_t bitPosition, uint8_t bitWidth, uint32_t value);

#endif /* INC_GENERAL_H_ */
