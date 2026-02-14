/*
 * board_imu.c
 *
 *  Created on: Jan 3, 2026
 *      Author: dobao
 */
#include <imu/imu.h>

/*
 * =============================================================================
 * 							DEFAULT CONSTANTS OF ICM42688P
 * =============================================================================
 */
/* Gyro Config Defaults */
const ICM42688_Gyro_Config_t GYRO_DEFAULT = {
		.gyro_odr			= GYRO_ODR_1KHz,
		.gyro_fsr			= GYRO_FSR_2000dps,
		.gyro_notch 		= GYRO_NOTCHBW_680Hz,
		.gyro_filt_order 	= GYRO_SECOND_ORDER,
		.gyro_mode			= GYRO_OFF,
};


/* Accel Config Defaults */
const ICM42688_Accel_Config_t ACCEL_DEFAULT = {
		.accel_odr			= ACCEL_ODR_1KHz,
		.accel_fsr			= ACCEL_FSR_16g,
		.accel_filt_order	= ACCEL_SECOND_ORDER,
		.accel_mode			= ACCEL_OFF,
};


/* SPI Config Default */
const ICM42688_SPI_Config_t SPI_DEFAULT = {
		.hspi			= &hspi1,
		.cs_port		= GPIOA,
		.cs_pin			= GPIO_PIN_4,
		.spi_mode		= SPI_MODE_0_3,
		.spi_slew_rate	= SPI_SR_2NS,
};


/* Interrupt Defaults */
const ICM42688_Int1_Config_t INT1_DEFAULT = {
		.int1_polarity	= INT_ACTIVE_LOW,
		.int1_drive		= INT_OPEN_DRAIN,
		.int1_mode		= INT_PUSHED,
};


const ICM42688_Int2_Config_t INT2_DEFAULT = {
		.int2_polarity	= INT_ACTIVE_LOW,
		.int2_drive		= INT_OPEN_DRAIN,
		.int2_mode		= INT_PUSHED,
};


/*
 * ===================================================================
 * 			GLOBAL INITIAL IDENTITIES/PROPERTIES OF ICM42688
 * ===================================================================
 */
/* @brief	Initially, this struct stores default values */
ICM42688_Handle_t ICM42688_Handle = {
		.gyro_config	= GYRO_DEFAULT,
		.accel_config	= ACCEL_DEFAULT,
		.spi_config		= SPI_DEFAULT,
		.int1_config	= INT1_DEFAULT,
		.int2_config	= INT2_DEFAULT,

		.gyro_dps_per_lsb 	= 0.0f,
		.accel_g_per_lsb 	= 0.0f,

		.is_initialized		= false,
		.is_reset			= false,
		.is_alive			= false
};


/*
 * =============================================================================
 * 								  INITIALIZE
 * =============================================================================
 */
/*
 * @brief	Initialize and reset ICM42688 Sensor to the default settings
 */
//HAL_StatusTypeDef ICM42688_Init(ICM42688_Handle_t* handle){
//	/* Sanity Checks */
//	if(!handle) return HAL_ERROR;
//
//	/* Check if the sensor is alive */
//	HAL_StatusTypeDef status = ICM42688_IsAlive(handle);
//	if(status != HAL_OK) return status;
//
//	/* Soft reset the whole sensor */
//	status = ICM42688_SoftReset(handle);
//	if(status != HAL_OK) return status;
//
//	/* Immediately FORCE set to bank 0 */
//	status = ICM42688_Set_RegBank(handle, REG_BANK_0);
//	if(status != HAL_OK) return status;
//
//	/* Recheck if the sensor is alive after reset */
//	status = ICM42688_IsAlive(handle);
//	if(status != HAL_OK) return status;
//
//	/* Configure SPI */
//	status = ICM42688_Set_SPI_Mode(handle, SPI_MODE_0_3);
//	if(status != HAL_OK) return status;
//	status = ICM42688_Set_SPI_SlewRate(handle, SPI_SR_2NS);
//	if(status != HAL_OK) return status;
//
//	/* Configure Gyro */
//	status = ICM42688_Set_GyroConfig(handle, GYRO_OFF, GYRO_ODR_8KHz, GYRO_FSR_1000dps);
//	if(status != HAL_OK) return status;
//
//	/* Configure Accel */
//	status = ICM42688_Set_AccelConfig(handle, ACCEL_OFF, ACCEL_ODR_8KHz, ACCEL_FSR_8g);
//	if(status != HAL_OK) return status;
//
//	/* Initialization is completed and update cache */
//	handle -> is_initialized = true;
//	handle -> is_reset = false;
//	return HAL_OK;
//}


