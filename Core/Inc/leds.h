/*
 * leds.h
 *
 *  Created on: Dec 21, 2025
 *      Author: dobao
 */

#ifndef INC_LEDS_H_
#define INC_LEDS_H_
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "stm32f4xx_hal.h"

/* Define PortC for LED */
#define LED_GPIO_PORT	GPIOC

typedef enum{
	LED_BLUE = GPIO_PIN_13,	//Active LOW
	/* ADD OTHER LEDS LATER	 */
}LED_Color_t;

typedef enum{
	LED_OFF = GPIO_PIN_RESET,
	LED_ON = GPIO_PIN_SET
}LED_Control_Status_t;

/* Function Prototypes */
void ledInit(LED_Color_t ledColor);
void ledControl(LED_Control_Status_t state, LED_Color_t ledColor);
void ledControlAlt(LED_Color_t ledColor);

#endif /* INC_LEDS_H_ */
