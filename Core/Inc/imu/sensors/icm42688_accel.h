/*
 * icm42688_accel.h
 *
 *  Created on: Mar 12, 2026
 *      Author: dobao
 */

#ifndef SRC_IMU_SENSORS_ICM42688_ACCEL_H_
#define SRC_IMU_SENSORS_ICM42688_ACCEL_H_

#include "imu/core/icm42688_registers.h"
#include "imu/core/icm42688_masks.h"
#include "imu/core/icm42688_types.h"
#include "imu/core/icm42688_rw.h"

HAL_StatusTypeDef
ICM42688_Set_AccelConfig(ICM42688_Handle_t *handle, ICM42688_AccelMode_t mode,
					ICM42688_AccelODR_t odr, ICM42688_AccelFSR_t fsr);

HAL_StatusTypeDef
ICM42688_Get_Accel_Mode(ICM42688_Handle_t *handle, uint8_t *modeInfo);

HAL_StatusTypeDef
ICM42688_Set_Accel_UIFilt_BW(ICM42688_Handle_t *handle, ICM42688_UIFilt_BW_t bw);

#endif /* SRC_IMU_SENSORS_ICM42688_ACCEL_H_ */
