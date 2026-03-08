/*
 * board_imu.c
 *
 *  Created on: Jan 3, 2026
 *      Author: dobao
 */
#include <imu/icm42688_main.h>

/*
 * =============================================================================
 * 							DEFAULT CONSTANTS OF ICM42688P
 * =============================================================================
 */
const ICM42688_Gyro_Config_t GYRO_DEFAULT = {
		.gyro_odr			= GYRO_ODR_1KHz,
		.gyro_fsr			= GYRO_FSR_2000dps,
		.gyro_notch 		= GYRO_NOTCHBW_680Hz,
		.gyro_filt_order 	= GYRO_SECOND_ORDER,
		.gyro_mode			= GYRO_OFF,
		.gyro_uifilt_bw		= BW_400Hz_ODR_DIV_4,
};


const ICM42688_Accel_Config_t ACCEL_DEFAULT = {
		.accel_odr			= ACCEL_ODR_1KHz,
		.accel_fsr			= ACCEL_FSR_16g,
		.accel_filt_order	= ACCEL_SECOND_ORDER,
		.accel_mode			= ACCEL_OFF,
		.accel_uifilt_bw	= BW_400Hz_ODR_DIV_4,
};


const ICM42688_SPI_Config_t SPI_DEFAULT = {
		.hspi			= &hspi1,
		.cs_port		= GPIOA,
		.cs_pin			= GPIO_PIN_4,
		.spi_mode		= SPI_MODE_0_3,
		.spi_slew_rate	= SPI_SR_2NS,
};


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


const ICM42688_Temp_Config_t TEMP_DEFAULT = {
		.temp_state = TEMP_ENABLE,
};


const ICM42688_FIFO_Config_t FIFO_DEFAULT = {
		.fifo_gyro_state	= FIFO_GAT_DISABLE,
		.fifo_accel_state	= FIFO_GAT_DISABLE,
		.fifo_temp_state	= FIFO_GAT_DISABLE,

		.fifo_wm_mode				= FIFO_WM_GREATER_THS_ONESHOT,
		.fifo_hires_state			= FIFO_HIRES_DISABLE,
		.fifo_partial_read_state	= FIFO_PARTIAL_READ_DISABLE,
};




/*
 * ===================================================================
 * 			GLOBAL INITIAL IDENTITIES/PROPERTIES OF ICM42688
 * ===================================================================
 */
/* @brief	Initially, this struct stores default values */
ICM42688_Handle_t ICM42688_HANDLE = {
		.spi_config		= SPI_DEFAULT,

		.gyro_config	= GYRO_DEFAULT,
		.accel_config	= ACCEL_DEFAULT,

		.int1_config	= INT1_DEFAULT,
		.int2_config	= INT2_DEFAULT,

		.gyro_dps_per_lsb 			= 0.0f,
		.gyro_lsb_per_dps_dtsheet	= 0.0f,

		.accel_g_per_lsb			= 0.0f,
		.accel_lsb_per_g_dtsheet	= 0.0f,

		.is_initialized		= false,
		.is_reset			= false,
		.is_icm42688_alive	= false,

		.temp_config		= TEMP_DEFAULT,

		.fifo_config 		= FIFO_DEFAULT,
};
