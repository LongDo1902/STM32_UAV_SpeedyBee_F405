/*
 * icm42688_accel.h
 *
 *  Created on: Mar 12, 2026
 *      Author: dobaolong
 */

#ifndef SRC_IMU_SENSORS_ICM42688_ACCEL_H_
#define SRC_IMU_SENSORS_ICM42688_ACCEL_H_

#include "imu/core/icm42688_masks.h"
#include "imu/core/icm42688_registers.h"
#include "imu/core/icm42688_rw.h"
#include "imu/core/icm42688_types.h"

/**
 * @brief   Configure accelerometer power mode, output data rate, and full-scale range.
 *          Power-mode transitions are applied before ODR/FSR changes and include the device settling delay
 * when required. The acceleration scale factor is refreshed only after the hardware write succeeds.
 * @param   pHandle  Pointer to the ICM42688 handle struct.
 * @param   mode     Desired accelerometer power mode.
 * @param   odr      Desired accelerometer output data rate.
 * @param   fsr      Desired accelerometer full-scale range.
 * @return  true when the complete configuration succeeds, otherwise false.
 */
bool ICM42688_Set_AccelConfig(ICM42688_Handle_t *pHandle, ICM42688_Accel_Mode_t mode,
                              ICM42688_Accel_ODR_t odr, ICM42688_Accel_FSR_t fsr);

/**
 * @brief   Read the current accelerometer power mode.
 *          The function decodes ACCEL_MODE from PWR_MGMT0 and returns the raw field value so callers can
 *          distinguish every hardware-defined mode.
 * @param   pHandle    Pointer to the ICM42688 handle struct.
 * @param   pModeInfo  Pointer to the returned raw mode value.
 * @return  true when the register read succeeds, otherwise false.
 */
bool ICM42688_Get_Accel_Mode(ICM42688_Handle_t *pHandle, uint8_t *pModeInfo);

/**
 * @brief   Configure the accelerometer UI low-pass filter bandwidth.
 *          The register field accepts only the bandwidth encodings supported by the device; reserved values
 * are rejected before any SPI write is attempted.
 * @param   pHandle          Pointer to the ICM42688 handle struct.
 * @param   uiFiltBandWidth  Desired UI filter bandwidth selection.
 * @return  true when the register update succeeds, otherwise false.
 */
bool ICM42688_Set_Accel_UIFilt_BW(ICM42688_Handle_t *pHandle, ICM42688_UIFilt_BW_t uiFiltBandWidth);

/**
 * @brief   Configure the accelerometer UI low-pass filter order.
 *          The selected order changes the latency and attenuation of the UI filter and is cached only after a
 *          successful register update.
 * @param   pHandle      Pointer to the ICM42688 handle struct.
 * @param   uiFiltOrder  Desired UI filter order.
 * @return  true when the register update succeeds, otherwise false.
 */
bool ICM42688_Set_Accel_UIFilt_Order(ICM42688_Handle_t *pHandle, ICM42688_Accel_UIFilt_Order_t uiFiltOrder);

/**
 * @brief   Enable or disable the accelerometer anti-alias filter.
 *          Disabling this hardware filter can allow out-of-band vibration to alias into sampled data, so it
 *          should normally remain enabled unless an external filtering strategy replaces it.
 * @param   pHandle         Pointer to the ICM42688 handle struct.
 * @param   antiAliasState  Desired anti-alias filter state.
 * @return  true when the register update succeeds, otherwise false.
 */
bool ICM42688_Set_Accel_Anti_Alias_Filt(ICM42688_Handle_t *pHandle, ICM42688_AAF_En_t antiAliasState);

#endif /* SRC_IMU_SENSORS_ICM42688_ACCEL_H_ */
