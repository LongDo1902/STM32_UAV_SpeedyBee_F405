/*
 * icm42688_data.h
 *
 *  Created on: Mar 14, 2026
 *      Author: dobao
 */

#ifndef INC_IMU_SENSORS_ICM42688_DATA_H_
#define INC_IMU_SENSORS_ICM42688_DATA_H_

#include "imu/core/icm42688_registers.h"
#include "imu/core/icm42688_masks.h"
#include "imu/core/icm42688_types.h"
#include "imu/core/icm42688_rw.h"

HAL_StatusTypeDef
ICM42688_Get_Temperature_C(ICM42688_Handle_t *handle, float *out_temp_c);

HAL_StatusTypeDef
ICM42688_Get_Accel_XYZ(ICM42688_Handle_t *handle, int16_t *buf);

HAL_StatusTypeDef
ICM42688_Get_Accel_G(ICM42688_Handle_t *handle, float g[3]);

HAL_StatusTypeDef
ICM42688_Get_Gyro_XYZ(ICM42688_Handle_t *handle, int16_t *buf);

HAL_StatusTypeDef
ICM42688_Get_Gyro_DPS(ICM42688_Handle_t *handle, float dps[3]);

#endif /* INC_IMU_SENSORS_ICM42688_DATA_H_ */
