/*
 * leds.c
 *
 *  Created on: Dec 21, 2025
 *      Author: dobao
 */

#include "leds.h"

/*
 * @brief	Initialize LED GPIOs
 */
void Long_ledInit(LED_Color_t ledColor){
	/* Enable HAL RCC Clock for GPIOC */
	__HAL_RCC_GPIOC_CLK_ENABLE();

	GPIO_InitTypeDef GPIO_InitStruct = {0};

	/* Configure the GPIO Pin */
	GPIO_InitStruct.Pin = ledColor;					//GPIO_PIN_13
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;		//Output Push Pull
	GPIO_InitStruct.Pull = GPIO_NOPULL;				//No Pull Up
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW; 	//IO works at 2MHz

	HAL_GPIO_Init(LED_GPIO_PORT, &GPIO_InitStruct);
}


/*
 * @brief	Control the specific LED state
 */
void Long_ledControl(LED_Control_Status_t state, LED_Color_t ledColor){
	/* LED_BLUE / PC13 is an active LOW LED so led state is flipped */
	HAL_GPIO_WritePin(LED_GPIO_PORT, ledColor, !(GPIO_PinState)state);
}


/*
 * @brief	Alternative controlling LED state
 */
void Long_ledControlAlt(LED_Color_t ledColor){
	HAL_GPIO_TogglePin(LED_GPIO_PORT, ledColor);
}


