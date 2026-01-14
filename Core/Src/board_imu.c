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
ICM42688_Handle_t ICM42688_Handle = {
		.config = {
				.gyro_odr			= GYRO_ODR_1KHz,
				.gyro_fsr			= GYRO_FSR_1000dps,
				.gyro_notch			= GYRO_NOTCHBW_11449Hz,
				.gyro_filt_order 	= GYRO_FIRST_ORDER,
				.gyro_mode			= GYRO_OFF,				//Default

				.accel_odr			= ACCEL_ODR_1KHz,
				.accel_fsr			= ACCEL_FSR_4g,
				.accel_filt_order	= ACCEL_FIRST_ORDER,
				.accel_mode			= ACCEL_OFF,			//Default

				.spiMode			= SPI_MODE_0_3,			//Default
				.spiSlewRate		= SPI_SR_2NS			//Default
		},

		.spi_io = {
				.hspi		= &hspi1,
				.cs_port 	= GPIOA,
				.cs_pin		= GPIO_PIN_4,
		},

		.gyro_lsb_to_dps 	= 0.0f,
		.accel_lsb_to_g 	= 0.0f,

		.isInitialized		= false,
		.isAlive			= false,
		.regBank			= REG_BANK_0	//Default
};




