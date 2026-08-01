/*
 * icm42688_interface.h
 *
 *  Created on: Mar 12, 2026
 *      Author: dobaolong
 */

#ifndef INC_IMU_ICM42688_INTERFACE_H_
#define INC_IMU_ICM42688_INTERFACE_H_

#include "imu/core/icm42688_masks.h"
#include "imu/core/icm42688_registers.h"
#include "imu/core/icm42688_rw.h"
#include "imu/core/icm42688_types.h"

/**
 * @brief   Configure the ICM42688 SPI clock and data phase mode.
 *          The selected mode is written to DEVICE_CONFIG and cached in the handle; an identical request is
 *          skipped after initialization to avoid an unnecessary register transaction.
 * @param   pHandle  Pointer to the ICM42688 handle struct.
 * @param   spiMode  Desired SPI mode selection.
 * @return  true when the register update succeeds, otherwise false.
 */
bool ICM42688_Set_SPI_Mode(ICM42688_Handle_t *pHandle, ICM42688_SPI_Mode_t spiMode);

/**
 * @brief   Read the configured SPI output slew rate.
 *          The raw DRIVE_CONFIG field is decoded into the driver enum and returned through pSlewRate without
 *          changing the cached configuration.
 * @param   pHandle    Pointer to the ICM42688 handle struct.
 * @param   pSlewRate  Pointer to the returned slew-rate selection.
 * @return  true when the register read succeeds, otherwise false.
 */
bool ICM42688_Get_SPI_SlewRate(ICM42688_Handle_t *pHandle, ICM42688_SPI_SLEWRATE_t *pSlewRate);

/**
 * @brief   Configure the SPI output slew rate.
 *          This controls the electrical edge rate of the SPI output driver and should match PCB loading and
 *          signal-integrity requirements rather than simply using the fastest setting.
 * @param   pHandle   Pointer to the ICM42688 handle struct.
 * @param   slewRate  Desired slew-rate selection.
 * @return  true when the register update succeeds, otherwise false.
 */
bool ICM42688_Set_SPI_SlewRate(ICM42688_Handle_t *pHandle, ICM42688_SPI_SLEWRATE_t slewRate);

/**
 * @brief   Configure which serial interfaces remain enabled for user access.
 *          The UI_SIFS_CFG field can disable I2C or SPI access, so the selected value must preserve the bus
 * used by the application or subsequent register transactions may no longer work.
 * @param   pHandle       Pointer to the ICM42688 handle struct.
 * @param   uiSifsConfig  Desired user-interface serial configuration.
 * @return  true when the register update succeeds, otherwise false.
 */
bool ICM42688_Set_UI_SIFS_Conf(ICM42688_Handle_t *pHandle, ICM42688_UI_SIFS_Cfg_t uiSifsConfig);

/**
 * @brief   Configure the byte order used by accelerometer, gyroscope, and temperature data.
 *          The choice is cached in the handle and is later used by raw-data decoding; changing it requires
 * all consumers to interpret multi-byte sensor fields with the same ordering.
 * @param   pHandle      Pointer to the ICM42688 handle struct.
 * @param   whichEndian  Desired sensor-data byte order.
 * @return  true when the register update succeeds, otherwise false.
 */
bool ICM42688_Set_Sensor_Data_Endian(ICM42688_Handle_t *pHandle, ICM42688_Sensor_Data_Endian_t whichEndian);

#endif /* INC_IMU_ICM42688_INTERFACE_H_ */
