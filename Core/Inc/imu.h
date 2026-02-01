/*
 * board_imu.h
 *
 *  Created on: Jan 3, 2026
 *      Author: dobao
 */

#ifndef INC_IMU_H_
#define INC_IMU_H_

#include <stdint.h>
#include <stdio.h>

#include "icm42688.h"

extern const ICM42688_Gyro_Config_t 	GYRO_DEFAULT;
extern const ICM42688_Accel_Config_t 	ACCEL_DEFAULT;
extern const ICM42688_SPI_Config_t 		SPI_DEFAULT;
extern const ICM42688_Int1_Config_t 	INT1_DEFAULT;
extern const ICM42688_Int2_Config_t 	INT2_DEFAULT;

#endif /* INC_IMU_H_ */
