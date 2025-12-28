/*
 * icm42688p.c
 *
 *  Created on: Dec 22, 2025
 *      Author: dobao
 */

#include "icm42688.h"

void ICM42688_Init(ICM42688_Handle_t *handle, ICM42688_Config_t config){
	/* Save the config to the handle */
	handle -> config = config;

	/* Pre-calculate the Gyro multiplier
	 * Calculation: (MAX DPS) / 32768.0 */
	float gyro_max_values[] = {2000
}
