/*
 * temperature.c
 *
 *  Created on: Dec 24, 2025
 *      Author: dobao
 */

#include "temperature.h"
#include "adc.h"

/*
 * ===========================================================
 * 						Global Variables
 * ===========================================================
 */
volatile float rawADCValueInt	= 0.0f;	//Raw ADC value retrieved from the triggered Interrupt
volatile float STM32Temperature = 0.0f; //Final temperature of STM32 MCU

/*
 * ===========================================================
 * 			  Public APIs for Getting Temperature
 * ===========================================================
 */
/*
 * @brief	Start ADC1 Interrupt
 */
void Long_ADC_startADC1Int(ADC_HandleTypeDef* hadc){
	(void)HAL_ADC_Start_IT(hadc);
}


/*
 * @brief	Interrupt callback
 */
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc){
	if(hadc -> Instance == ADC1){
		rawADCValueInt = (float)HAL_ADC_GetValue(hadc);	//Read data
		if(TEMPERATURE_AS_INTEGER){
			STM32Temperature = (int32_t)Long_ADC_getSTM32Temperature();	//Extract temperature as Integer
		}
		else{
			STM32Temperature = (float)Long_ADC_getSTM32Temperature();	//Extract temperature as Float
		}
		HAL_ADC_Start_IT(hadc);	//Restart the interrupt to ask for more new data
	}
}


/*
 * @brief	Calculate STM32's temperature
 */
float Long_ADC_getSTM32Temperature(){
	float temperature = 0.0f;

	/* Convert ADC value to milli voltage */
	float ADC_to_Voltage = ((float)(rawADCValueInt / ADC_MAX)) * VREF;

	/*
	 * Calculate temperature by:
	 * 		1.	Calculate the difference between current-actual milli-voltage and
	 * 			constant milli-voltage at 25*C
	 * 		2.	Them, divide it by how much voltage changes for every 1*C.
	 * 		3.	Finally, add a constant of 25*C and the "fine-tune offset"
	 */
	return temperature = (float)(((ADC_to_Voltage - VOLTAGE_AT_25DEG) / AVG_SLOPE) + (float)25.0 + OFFSET_TEMPERATURE);
}


s















