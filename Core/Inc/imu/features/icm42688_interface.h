/*
 * icm42688_interface.h
 *
 *  Created on: Mar 12, 2026
 *      Author: dobao
 */

#ifndef INC_IMU_ICM42688_INTERFACE_H_
#define INC_IMU_ICM42688_INTERFACE_H_

#include "imu/core/icm42688_registers.h"
#include "imu/core/icm42688_masks.h"
#include "imu/core/icm42688_types.h"
#include "imu/core/icm42688_rw.h"

HAL_StatusTypeDef ICM42688_Set_SPI_Mode(ICM42688_Handle_t* handle, ICM42688_SPI_Mode_t spiMode);

HAL_StatusTypeDef ICM42688_Get_SPI_SlewRate(ICM42688_Handle_t* handle, ICM42688_SPI_SLEWRATE_t* slewRate);

HAL_StatusTypeDef ICM42688_Set_SPI_SlewRate(ICM42688_Handle_t* handle, ICM42688_SPI_SLEWRATE_t slewRate);

HAL_StatusTypeDef ICM42688_Set_UI_SIFS_Conf(ICM42688_Handle_t* handle, ICM42688_UI_SIFS_Cfg_t config);

HAL_StatusTypeDef ICM42688_Set_Sensor_Data_Endian(ICM42688_Handle_t* handle, ICM42688_Sensor_Data_Endian_t which_endian);

#endif /* INC_IMU_ICM42688_INTERFACE_H_ */
