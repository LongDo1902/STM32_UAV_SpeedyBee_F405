/*
 * temperature.h
 *
 *  Created on: Dec 24, 2025
 *      Author: dobao
 */

#ifndef INC_TEMPERATURE_H_
#define INC_TEMPERATURE_H_

#include <stdio.h>
#include <stdint.h>

#include "adc.h"

extern volatile float rawADCValueInt;
extern volatile float STM32Temperature;


/*
 * ===========================================================
 * 					Configuration Choices
 * ===========================================================
 */
#define TEMPERATURE_AS_INTEGER	1U	//Set 1 if you want the value temperature is an integer

/*
 * ===========================================================
 * 					MCU's Temperature Constants
 * ===========================================================
 */
#define VOLTAGE_AT_25DEG	760.0f	//How much milli-voltage that sensor output when the chip is at 25*C
#define	AVG_SLOPE			2.5f	//How much voltage changes for every 1*C change in temperature
#define VREF				3300.0f	//System reference voltage 3V3
#define ADC_MAX				4094.0f	//The maximum possible value of a 12-bit value can output
#define	OFFSET_TEMPERATURE	5.0f	//Modify/tune this number to match the actual temperature of the chip


/*
 * ===========================================================
 * 							Public APIs
 * ===========================================================
 */
void Long_ADC_startADC1Int(ADC_HandleTypeDef* hadc);
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc);
float Long_ADC_getSTM32Temperature();

#endif /* INC_TEMPERATURE_H_ */
