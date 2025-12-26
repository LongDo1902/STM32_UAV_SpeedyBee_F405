/*
 * icm42688p.h
 *
 *  Created on: Dec 22, 2025
 *      Author: dobao
 */

#ifndef INC_ICM42688P_H_
#define INC_ICM42688P_H_

#include <stdio.h>
#include <stdint.h>

/*
 * =============================================================
 * 						ICM42688-P Macros
 * =============================================================
 */
#define ICM42688P_VDD			1800U	//In mV

/* Frequencies are declared in Hz */
#define ICM42688P_SPI_MAX_CLKFREQ	24000000U
#define ICM42688P_I2C_MAX_CLKFREQ	1000000U
#define ICM42688P_MAX_RTC_FREQ		50000U
#define ICM42688P_TYP_RTC_FREQ		32000U
#define ICM42688P_MIN_RTC_FREQ		31000U



#endif /* INC_ICM42688P_H_ */
