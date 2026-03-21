/*
 * icm42688_defs.h
 *
 *  Created on: Mar 12, 2026
 *      Author: dobao
 */

#ifndef INC_IMU_ICM42688_DEFS_H_
#define INC_IMU_ICM42688_DEFS_H_

/* =============================================================
 *	ICM42688-P MACROS
 * ============================================================= */

#define ICM42688_VDD				1800U	// In mV

// Frequencies are declared in Hz
#define ICM42688_SPI_MAX_CLKFREQ	24000000U
#define ICM42688_I2C_MAX_CLKFREQ	1000000U
#define ICM42688_MAX_RTC_FREQ		50000U
#define ICM42688_TYP_RTC_FREQ		32000U
#define ICM42688_MIN_RTC_FREQ		31000U

// Other Defines
#define ICM42688_SENSITIVITY_SCALE_FACTOR		32768.0f
#define ICM42688_SPI_TIMEOUT_MS			20U
#define ICM42688_SPI_ADDR_MASK			0x7FU
#define ICM42688_SPI_READ_BIT				0x80U

#endif /* INC_IMU_ICM42688_DEFS_H_ */
