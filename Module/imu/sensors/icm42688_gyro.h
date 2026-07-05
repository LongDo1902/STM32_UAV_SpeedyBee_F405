/*
 * icm42688_gyro.h
 *
 *  Created on: Mar 12, 2026
 *      Author: dobao
 */

#ifndef SRC_IMU_SENSORS_ICM42688_GYRO_H_
#define SRC_IMU_SENSORS_ICM42688_GYRO_H_

#include "imu/core/icm42688_masks.h"
#include "imu/core/icm42688_registers.h"
#include "imu/core/icm42688_rw.h"
#include "imu/core/icm42688_types.h"

bool
ICM42688_Set_GyroConfig(ICM42688_Handle_t *handle, ICM42688_Gyro_Mode_t mode, ICM42688_Gyro_ODR_t odr,
                        ICM42688_Gyro_FSR_t fsr);

bool
ICM42688_Get_Gyro_Mode(ICM42688_Handle_t *handle, uint8_t *modeInfo);

bool
ICM42688_Set_Gyro_UIFilt_BW(ICM42688_Handle_t *handle, ICM42688_UIFilt_BW_t uiFiltBandWidth);

bool
ICM42688_Set_Gyro_UIFilt_Order(ICM42688_Handle_t *handle, ICM42688_Gyro_UIFilt_Order_t uiFiltOrder);

bool
ICM42688_Set_Gyro_Anti_Alias_Filt(ICM42688_Handle_t *handle, ICM42688_AAF_En_t antiAliasState);

bool
ICM42688_Set_Gyro_Notch_Filt(ICM42688_Handle_t *handle, ICM42688_Notch_Filt_En_t notchFiltState);

bool
ICM42688_Set_NotchFreq_XYZ(ICM42688_Handle_t *handle, uint16_t desiredXNotchFreqHz,
                           uint16_t desiredYNotchFreqHz, uint16_t desiredZNotchFreqHz);

#endif /* SRC_IMU_SENSORS_ICM42688_GYRO_H_ */
