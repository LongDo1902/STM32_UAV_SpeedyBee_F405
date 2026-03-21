/*
 * icm42688_device.h
 *
 *  Created on: Mar 12, 2026
 *      Author: dobao
 */

#ifndef INC_IMU_ICM42688_DEVICE_H_
#define INC_IMU_ICM42688_DEVICE_H_

#include "imu/core/icm42688_registers.h"
#include "imu/core/icm42688_masks.h"
#include "imu/core/icm42688_types.h"
#include "imu/core/icm42688_rw.h"

#define ICM42688_WHO_AM_I_DEFAULT	0x47U

HAL_StatusTypeDef
ICM42688_IsAlive(ICM42688_Handle_t *handle);

HAL_StatusTypeDef
ICM42688_SoftReset(ICM42688_Handle_t *handle);

#endif /* INC_IMU_ICM42688_DEVICE_H_ */
