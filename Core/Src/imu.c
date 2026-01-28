/*
 * board_imu.c
 *
 *  Created on: Jan 3, 2026
 *      Author: dobao
 */
#include <imu.h>


/*
 * =============================================================================
 * 								PRIVATE CONSTANTS
 * =============================================================================
 */
/* Register-field code -> physical full-scale value mapping */
static const float gyro_FSR_value[]		= {2000.0f, 1000.0f, 500.0f, 250.0f, 125.0f, 65.5f, 31.25f, 15.625f};
static const float accel_FSR_value[]	= {16.0f, 8.0f, 4.0f, 2.0f};

/*
 * =============================================================================
 * 							DEFAULT CONSTANTS OF ICM42688P
 * =============================================================================
 */
/* Gyro Config Defaults */
static const ICM42688_Gyro_Config_t GYRO_DEFAULT = {
		.gyro_odr			= GYRO_ODR_1KHz,
		.gyro_fsr			= GYRO_FSR_2000dps,
		.gyro_notch 		= GYRO_NOTCHBW_680Hz,
		.gyro_filt_order 	= GYRO_SECOND_ORDER,
		.gyro_mode			= GYRO_OFF,
};


/* Accel Config Defaults */
static const ICM42688_Accel_Config_t ACCEL_DEFAULT = {
		.accel_odr			= ACCEL_ODR_1KHz,
		.accel_fsr			= ACCEL_FSR_16g,
		.accel_filt_order	= ACCEL_SECOND_ORDER,
		.accel_mode			= ACCEL_OFF,
};


/* Interrupt Defaults */
static const ICM42688_Int1_Config_t INT1_DEFAULT = {
		.int1_polarity	= INT_ACTIVE_LOW,
		.int1_drive		= INT_OPEN_DRAIN,
		.int1_mode		= INT_PUSHED,
};


static const ICM42688_Int2_Config_t INT2_DEFAULT = {
		.int2_polarity	= INT_ACTIVE_LOW,
		.int2_drive		= INT_OPEN_DRAIN,
		.int2_mode		= INT_PUSHED,
};


/*
 * ===================================================================
 * 			GLOBAL INITIAL IDENTITIES/PROPERTIES OF ICM42688
 * ===================================================================
 */
ICM42688_Handle_t ICM42688_Handle = {
		/* Gyro & Accel */
		.gyro_config = {
				.gyro_odr			= GYRO_DEFAULT.gyro_odr,
				.gyro_fsr			= GYRO_DEFAULT.gyro_fsr,
				.gyro_notch			= GYRO_DEFAULT.gyro_notch,
				.gyro_filt_order 	= GYRO_DEFAULT.gyro_filt_order,
				.gyro_mode			= GYRO_DEFAULT.gyro_mode,
		},

		.accel_config = {
				.accel_odr			= ACCEL_DEFAULT.accel_odr,
				.accel_fsr			= ACCEL_DEFAULT.accel_fsr,
				.accel_filt_order	= ACCEL_DEFAULT.accel_filt_order,
				.accel_mode			= ACCEL_DEFAULT.accel_mode,
		},

		.spi_config = {
				.hspi			= &hspi1,
				.cs_port 		= GPIOA,
				.cs_pin			= GPIO_PIN_4,
				.spiMode 		= SPI_MODE_0_3,		//Default
				.spiSlewRate	= SPI_SR_2NS,		//Default
		},

		.int1_config = {
				.int1_polarity	= INT1_DEFAULT.int1_polarity,
				.int1_drive		= INT1_DEFAULT.int1_drive,
				.int1_mode		= INT1_DEFAULT.int1_mode,
		},

		.int2_config = {
				.int2_polarity	= INT2_DEFAULT.int2_polarity,
				.int2_drive		= INT2_DEFAULT.int2_drive,
				.int2_mode		= INT2_DEFAULT.int2_mode,
		},

		.gyro_lsb_to_dps 	= 0.0f,
		.accel_lsb_to_g 	= 0.0f,

		.isInitialized		= false,
		.isAlive			= false,
		.regBank			= REG_BANK_0,	//Default
};


/*
 * =============================================================================
 * 								  INITIALIZE
 * =============================================================================
 */
/*
 * @brief	Initialize and reset ICM42688 Sensor to the default settings
 */
HAL_StatusTypeDef ICM42688_Init(ICM42688_Handle_t* handle){
	/* Sanity Checks */
	if(!handle) return HAL_ERROR;

	/* Check if the sensor is alive */
	if(ICM42688_IsAlive(handle) != HAL_OK) return HAL_ERROR;

	/* Soft reset the whole sensor */
	if(ICM42688_SoftReset(handle) != HAL_OK) return HAL_ERROR;

	/* Immediately FORCE set to bank 0 */
	if(ICM42688_Set_RegBank(handle, REG_BANK_0) != HAL_OK) return HAL_ERROR;

	/* Recheck if the sensor is alive after reset */
	if(ICM42688_IsAlive(handle) != HAL_OK) return HAL_ERROR;

	/* Force the sensors OFF */
	if(ICM42688_Set_GyroMode(handle, GYRO_DEFAULT.gyro_mode) != HAL_OK) return HAL_ERROR;
	if(ICM42688_Set_AccelMode(handle, ACCEL_DEFAULT.accel_mode) != HAL_OK) return HAL_ERROR;

	/* Configure Default */
	if(ICM42688_Set_SPI_Mode(handle, SPI_MODE_0_3) != HAL_OK) return HAL_ERROR;
	if(ICM42688_Set_SPI_SlewRate(handle, SPI_SR_2NS) != HAL_OK) return HAL_ERROR;

	/* Configure Gyro */
	ICM42688_Set_GyroODR(handle, GYRO_DEFAULT.gyro_odr);
	ICM42688_Set_GyroFS(handle, GYRO_DEFAULT.gyro_fsr);

	/* Configure Accel */
	ICM42688_Set_AccelODR(handle, ACCEL_DEFAULT.accel_odr);
	ICM42688_Set_AccelFS(handle, ACCEL_DEFAULT.accel_fsr);

	/* Precalculate sensitivity multiplier of Gyro and Accel
	 * Formula:	MAX FSR / 32768.0 */
	handle -> gyro_lsb_to_dps	= (float)(gyro_FSR_value[handle -> gyro_config.gyro_fsr] / ICM42688_SENSITIVITY_SCALE_FACTOR);
	handle -> accel_lsb_to_g	= (float)(accel_FSR_value[handle -> accel_config.accel_fsr] / ICM42688_SENSITIVITY_SCALE_FACTOR);

	handle -> isInitialized = true;
	return HAL_OK;
}


