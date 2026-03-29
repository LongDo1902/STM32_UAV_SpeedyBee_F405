/*
 * icm42688_gyro.h
 *
 *  Created on: Mar 12, 2026
 *      Author: dobao
 */

#ifndef SRC_IMU_SENSORS_ICM42688_GYRO_H_
#define SRC_IMU_SENSORS_ICM42688_GYRO_H_

#include "imu/core/icm42688_registers.h"
#include "imu/core/icm42688_masks.h"
#include "imu/core/icm42688_types.h"
#include "imu/core/icm42688_rw.h"

HAL_StatusTypeDef
ICM42688_Set_GyroConfig(ICM42688_Handle_t *handle, ICM42688_GyroMode_t mode, ICM42688_GyroODR_t odr,
				ICM42688_GyroFSR_t fsr);

HAL_StatusTypeDef
ICM42688_Get_Gyro_Mode(ICM42688_Handle_t *handle, uint8_t *modeInfo);

HAL_StatusTypeDef
ICM42688_Set_Gyro_UIFilt_BW(ICM42688_Handle_t *handle, ICM42688_UIFilt_BW_t bw);

HAL_StatusTypeDef
ICM42688_Set_Gyro_UIFilt_Order(ICM42688_Handle_t *handle, ICM42688_GyroUIFiltOrder_t filterOrder);

HAL_StatusTypeDef
ICM42688_Set_Gyro_Anti_Alias_Filt(ICM42688_Handle_t *handle, ICM42688_AAF_En_t antiAliasState);

#endif /* SRC_IMU_SENSORS_ICM42688_GYRO_H_ */
