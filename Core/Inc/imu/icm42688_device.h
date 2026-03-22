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

#include "imu/features/icm42688_interface.h"
#include "imu/features/icm42688_interrupt.h"

#include "imu/sensors/icm42688_accel.h"
#include "imu/sensors/icm42688_gyro.h"
#include "imu/sensors/icm42688_temp.h"
#include "imu/sensors/icm42688_data.h"

#define ICM42688_WHO_AM_I_DEFAULT	0x47U

HAL_StatusTypeDef
ICM42688_IsAlive(ICM42688_Handle_t *handle);

HAL_StatusTypeDef
ICM42688_SoftReset(ICM42688_Handle_t *handle);

#endif /* INC_IMU_ICM42688_DEVICE_H_ */
