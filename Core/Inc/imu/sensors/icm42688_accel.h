/*
 * icm42688_accel.h
 *
 *  Created on: Mar 12, 2026
 *      Author: dobao
 */

#ifndef SRC_IMU_SENSORS_ICM42688_ACCEL_H_
#define SRC_IMU_SENSORS_ICM42688_ACCEL_H_

#include "imu/core/icm42688_masks.h"
#include "imu/core/icm42688_registers.h"
#include "imu/core/icm42688_rw.h"
#include "imu/core/icm42688_types.h"

ICM42688_Status_t
ICM42688_Set_AccelConfig(ICM42688_Handle_t *handle, ICM42688_Accel_Mode_t mode,
                         ICM42688_Accel_ODR_t odr, ICM42688_Accel_FSR_t fsr);

ICM42688_Status_t
ICM42688_Get_Accel_Mode(ICM42688_Handle_t *handle, uint8_t *modeInfo);

ICM42688_Status_t
ICM42688_Set_Accel_UIFilt_BW(ICM42688_Handle_t *handle, ICM42688_UIFilt_BW_t bw);

ICM42688_Status_t
ICM42688_Set_Accel_UIFilt_Order(ICM42688_Handle_t *handle, ICM42688_Accel_UIFilt_Order_t filterOrder);

ICM42688_Status_t
ICM42688_Set_Accel_Anti_Alias_Filt(ICM42688_Handle_t *handle, ICM42688_AAF_En_t antiAliasState);

#endif /* SRC_IMU_SENSORS_ICM42688_ACCEL_H_ */
