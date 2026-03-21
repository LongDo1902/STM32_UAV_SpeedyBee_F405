/*
 * icm42688_temp.h
 *
 *  Created on: Mar 12, 2026
 *      Author: dobao
 */

#ifndef SRC_IMU_SENSORS_ICM42688_TEMP_H_
#define SRC_IMU_SENSORS_ICM42688_TEMP_H_

#include "imu/core/icm42688_registers.h"
#include "imu/core/icm42688_masks.h"
#include "imu/core/icm42688_types.h"
#include "imu/core/icm42688_rw.h"

HAL_StatusTypeDef
ICM42688_Set_Temperature_Enable(ICM42688_Handle_t *handle, ICM42688_Temp_t state);

#endif /* SRC_IMU_SENSORS_ICM42688_TEMP_H_ */
