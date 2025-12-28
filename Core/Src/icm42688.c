/*
 * icm42688p.c
 *
 *  Created on: Dec 22, 2025
 *      Author: dobao
 */

#include "icm42688.h"

void ICM42688_Init(ICM42688_Handle_t *handle, ICM42688_Config_t config){
	/* Save the config into "handle" */
	handle -> config = config;

	/* Precalculate sensitivity multiplier of Gyro and Accel
	 * Formula:	MAX FSR / 32768.0 */
	float gyro_FSR_value[] = {2000.0f, 1000.0f, 500.0f, 250.0f, 125.0f, 65.5f, 31.25f, 15.625f};
	handle -> gyro_lsb_to_dps = (float)(gyro_FSR_value[config.gyro_fsr] / ICM42688_SENSITIVITY_SCALE_FACTOR);

	float accel_FSR_value[] = {16.0f, 8.0f, 4.0f, 2.0f};
	handle -> accel_lsb_to_dps = (float)(accel_FSR_value[config.accel_fsr] / ICM42688_SENSITIVITY_SCALE_FACTOR);
}
