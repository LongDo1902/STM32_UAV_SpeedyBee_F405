/*
 * board_imu.c
 *
 *  Created on: Jan 3, 2026
 *      Author: dobao
 */
#include "board_imu.h"

/*
 * ===================================================================
 * 			GLOBAL INITIAL IDENTITIES/PROPERTIES OF ICM42688
 * ===================================================================
 */
ICM42688_Config_t ICM42688_Config = {
		.gyro_odr 			= GYRO_ODR_1KHz,
		.gyro_fsr 			= GYRO_FSR_1000dps,
		.gyro_notch 		= GYRO_NOTCHBW_11449Hz,
		.gyro_filt_order 	= GYRO_FIRST_ORDER,

		.accel_odr 			= ACCEL_ODR_1KHz,
		.accel_fsr 			= ACCEL_FSR_4g,
		.accel_filt_order 	= ACCEL_FIRST_ORDER
};

ICM42688_Handle_t ICM42688_Handle = {
		.gyro_lsb_to_dps 	= 0.0,
		.accel_lsb_to_g 	= 0.0,

		.hspi				= &hspi1,
		.cs_port			= GPIOA,
		.cs_pin				= GPIO_PIN_4,

		.regBank 			= REG_BANK_0
};


