/*
 * icm42688_gyro.h
 *
 *  Created on: Mar 12, 2026
 *      Author: dobaolong
 */

#ifndef SRC_IMU_SENSORS_ICM42688_GYRO_H_
#define SRC_IMU_SENSORS_ICM42688_GYRO_H_

#include "imu/core/icm42688_masks.h"
#include "imu/core/icm42688_registers.h"
#include "imu/core/icm42688_rw.h"
#include "imu/core/icm42688_types.h"

/**
 * @brief   Configure gyroscope power mode, output data rate, and full-scale range.
 *          Power-mode transitions are applied before ODR/FSR changes and include the device settling delay
 * when required. The angular-rate scale factor is refreshed only after the hardware write succeeds.
 * @param   pHandle  Pointer to the ICM42688 handle struct.
 * @param   mode     Desired gyroscope power mode.
 * @param   odr      Desired gyroscope output data rate.
 * @param   fsr      Desired gyroscope full-scale range.
 * @return  true when the complete configuration succeeds, otherwise false.
 */
bool ICM42688_Set_GyroConfig(ICM42688_Handle_t *pHandle, ICM42688_Gyro_Mode_t mode, ICM42688_Gyro_ODR_t odr,
                             ICM42688_Gyro_FSR_t fsr);

/**
 * @brief   Read the current gyroscope power mode.
 *          The function decodes GYRO_MODE from PWR_MGMT0 and validates the returned field against supported
 *          operating modes before reporting success.
 * @param   pHandle    Pointer to the ICM42688 handle struct.
 * @param   pModeInfo  Pointer to the returned raw mode value.
 * @return  true when the register read succeeds, otherwise false.
 */
bool ICM42688_Get_Gyro_Mode(ICM42688_Handle_t *pHandle, uint8_t *pModeInfo);

/**
 * @brief   Configure the gyroscope UI low-pass filter bandwidth.
 *          Reserved bandwidth encodings are rejected before the register update, preventing an undocumented
 *          filter response from being selected accidentally.
 * @param   pHandle          Pointer to the ICM42688 handle struct.
 * @param   uiFiltBandWidth  Desired UI filter bandwidth selection.
 * @return  true when the register update succeeds, otherwise false.
 */
bool ICM42688_Set_Gyro_UIFilt_BW(ICM42688_Handle_t *pHandle, ICM42688_UIFilt_BW_t uiFiltBandWidth);

/**
 * @brief   Configure the gyroscope UI low-pass filter order.
 *          The chosen order trades response latency for attenuation and is stored in the handle only after
 * the device accepts the register write.
 * @param   pHandle      Pointer to the ICM42688 handle struct.
 * @param   uiFiltOrder  Desired UI filter order.
 * @return  true when the register update succeeds, otherwise false.
 */
bool ICM42688_Set_Gyro_UIFilt_Order(ICM42688_Handle_t *pHandle, ICM42688_Gyro_UIFilt_Order_t uiFiltOrder);

/**
 * @brief   Enable or disable the gyroscope anti-alias filter.
 *          Disabling this hardware filter can fold high-frequency motor vibration into the measured band, so
 * it should be done only when the complete sampling and filtering chain has been considered.
 * @param   pHandle         Pointer to the ICM42688 handle struct.
 * @param   antiAliasState  Desired anti-alias filter state.
 * @return  true when the register update succeeds, otherwise false.
 */
bool ICM42688_Set_Gyro_Anti_Alias_Filt(ICM42688_Handle_t *pHandle, ICM42688_AAF_En_t antiAliasState);

/**
 * @brief   Enable or disable the gyroscope notch filter.
 *          This switch controls notch processing globally; configure valid per-axis center frequencies before
 *          enabling it to avoid filtering at stale or default frequency values.
 * @param   pHandle         Pointer to the ICM42688 handle struct.
 * @param   notchFiltState  Desired notch-filter state.
 * @return  true when the register update succeeds, otherwise false.
 */
bool ICM42688_Set_Gyro_Notch_Filt(ICM42688_Handle_t *pHandle, ICM42688_Notch_Filt_En_t notchFiltState);

/**
 * @brief   Configure the gyroscope notch-filter center frequency for all three axes.
 *          Each requested frequency is converted to the device coefficient representation and written to
 * bank 1. The operation returns false immediately if any axis conversion or register update fails.
 * @param   pHandle                 Pointer to the ICM42688 handle struct.
 * @param   desiredXNotchFreqHz     Desired X-axis notch frequency in hertz.
 * @param   desiredYNotchFreqHz     Desired Y-axis notch frequency in hertz.
 * @param   desiredZNotchFreqHz     Desired Z-axis notch frequency in hertz.
 * @return  true when all three axis configurations succeed, otherwise false.
 */
bool ICM42688_Set_NotchFreq_XYZ(ICM42688_Handle_t *pHandle, uint16_t desiredXNotchFreqHz,
                                uint16_t desiredYNotchFreqHz, uint16_t desiredZNotchFreqHz);

#endif /* SRC_IMU_SENSORS_ICM42688_GYRO_H_ */
