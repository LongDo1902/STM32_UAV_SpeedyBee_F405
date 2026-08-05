/*
 * icm42688_temp.h
 *
 *  Created on: Mar 12, 2026
 *      Author: dobaolong
 */

#ifndef SRC_IMU_SENSORS_ICM42688_TEMP_H_
#define SRC_IMU_SENSORS_ICM42688_TEMP_H_

#include "imu/core/icm42688_masks.h"
#include "imu/core/icm42688_registers.h"
#include "imu/core/icm42688_rw.h"
#include "imu/core/icm42688_types.h"

/**
 * @brief   Enable or disable the ICM42688 temperature sensor.
 *          The function updates the TEMP_DIS field in PWR_MGMT0 and caches the requested state only after the
 *          register write succeeds. FIFO temperature payload selection is configured separately.
 * @param   pHandle  Pointer to the ICM42688 handle struct.
 * @param   state    Desired temperature-sensor state.
 * @return  true when the register update succeeds, otherwise false.
 */
bool ICM42688_Set_Temperature_Enable(ICM42688_Handle_t *pHandle, ICM42688_Temp_t state);

#endif /* SRC_IMU_SENSORS_ICM42688_TEMP_H_ */
