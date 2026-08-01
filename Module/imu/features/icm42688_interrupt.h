/*
 * icm42688_interrupt.h
 *
 *  Created on: Mar 13, 2026
 *      Author: dobaolong
 */

#ifndef INC_IMU_ICM42688_INTERRUPT_H_
#define INC_IMU_ICM42688_INTERRUPT_H_

#include "imu/core/icm42688_masks.h"
#include "imu/core/icm42688_registers.h"
#include "imu/core/icm42688_rw.h"
#include "imu/core/icm42688_types.h"

/**
 * @brief   Configure the polarity, drive circuit, and signaling mode of INT1.
 *          All three INT_CONFIG fields are updated in one read-modify-write operation, then cached together
 * so software state changes only when the complete register transaction succeeds.
 * @param   pHandle   Pointer to the ICM42688 handle struct.
 * @param   polarity  Desired interrupt polarity.
 * @param   drive     Desired push-pull or open-drain drive circuit.
 * @param   mode      Desired pulsed or latched signaling mode.
 * @return  true when the register update succeeds, otherwise false.
 */
bool ICM42688_Set_Int1_Config(ICM42688_Handle_t *pHandle, ICM42688_Int_Polarity_t polarity,
                              ICM42688_Int_Drive_Circuit_t drive, ICM42688_Int_Mode_t mode);

/**
 * @brief   Configure the polarity, drive circuit, and signaling mode of INT2.
 *          All three INT_CONFIG fields are updated atomically at register level. Ensure the selected polarity
 *          and drive type agree with the MCU GPIO electrical configuration.
 * @param   pHandle   Pointer to the ICM42688 handle struct.
 * @param   polarity  Desired interrupt polarity.
 * @param   drive     Desired push-pull or open-drain drive circuit.
 * @param   mode      Desired pulsed or latched signaling mode.
 * @return  true when the register update succeeds, otherwise false.
 */
bool ICM42688_Set_Int2_Config(ICM42688_Handle_t *pHandle, ICM42688_Int_Polarity_t polarity,
                              ICM42688_Int_Drive_Circuit_t drive, ICM42688_Int_Mode_t mode);

/**
 * @brief   Read the ICM42688 interrupt status register.
 *          The returned byte is a snapshot containing all currently reported sources and can be passed to
 *          ICM42688_Int_Status_Has() to test individual events.
 * @param   pHandle     Pointer to the ICM42688 handle struct.
 * @param   pOutStatus  Pointer to the returned interrupt status bits.
 * @return  true when the register read succeeds, otherwise false.
 */
bool ICM42688_Get_Int_Status(ICM42688_Handle_t *pHandle, uint8_t *pOutStatus);

/**
 * @brief   Test whether a specific interrupt source is present in a status value.
 *          This is a local bit-mask check and performs no SPI transaction, making it suitable for decoding a
 *          previously captured status snapshot multiple times.
 * @param   status    Interrupt status value returned by ICM42688_Get_Int_Status().
 * @param   intState  Interrupt source to test.
 * @return  true when the requested interrupt bit is set, otherwise false.
 */
bool ICM42688_Int_Status_Has(uint8_t status, ICM42688_Int_Status_t intState);

/**
 * @brief   Enable or disable routing FIFO-full events to INT1.
 *          The function changes only the FIFO_FULL_INT1_EN source bit and preserves the routing state of
 * every other interrupt source in INT_SOURCE0.
 * @param   pHandle  Pointer to the ICM42688 handle struct.
 * @param   enable   true to enable the route, false to disable it.
 * @return  true when the register update succeeds, otherwise false.
 */
bool ICM42688_Set_Int1_FIFO_Full_Enable(ICM42688_Handle_t *pHandle, bool enable);

/**
 * @brief   Enable or disable routing FIFO-threshold events to INT1.
 *          A threshold route is useful only after the FIFO watermark and interrupt pin behavior are
 * configured; otherwise INT1 may remain inactive or assert with unintended timing.
 * @param   pHandle  Pointer to the ICM42688 handle struct.
 * @param   enable   true to enable the route, false to disable it.
 * @return  true when the register update succeeds, otherwise false.
 */
bool ICM42688_Set_Int1_FIFO_Threshold_Enable(ICM42688_Handle_t *pHandle, bool enable);

/**
 * @brief   Enable or disable routing data-ready events to INT1.
 *          This source can assert at the sensor ODR and may be much faster than a FIFO-watermark interrupt,
 * so enable it only when the ISR and downstream processing can tolerate that rate.
 * @param   pHandle  Pointer to the ICM42688 handle struct.
 * @param   enable   true to enable the route, false to disable it.
 * @return  true when the register update succeeds, otherwise false.
 */
bool ICM42688_Set_Int1_DataReady_Enable(ICM42688_Handle_t *pHandle, bool enable);

/**
 * @brief   Enable or disable routing reset-done events to INT1.
 *          The route is independent of polling INT_STATUS for reset completion and is normally disabled after
 *          initialization unless the application explicitly handles reset events on INT1.
 * @param   pHandle  Pointer to the ICM42688 handle struct.
 * @param   enable   true to enable the route, false to disable it.
 * @return  true when the register update succeeds, otherwise false.
 */
bool ICM42688_Set_Int1_ResetDone_Enable(ICM42688_Handle_t *pHandle, bool enable);

/**
 * @brief   Configure asynchronous reset, TDE assertion, and pulse duration behavior.
 *          The three INT_CONFIG1 controls are written together, so callers should supply the intended value
 * for every field rather than assuming unspecified settings will be preserved.
 * @param   pHandle              Pointer to the ICM42688 handle struct.
 * @param   setAsyncReset        Desired asynchronous-reset setting.
 * @param   setTdeAssertDisable  Desired TDE-assert-disable setting.
 * @param   setTpulseDuration    Desired interrupt pulse-duration setting.
 * @return  true when the register update succeeds, otherwise false.
 */
bool ICM42688_Set_INT_CONFIG1(ICM42688_Handle_t *pHandle, bool setAsyncReset, bool setTdeAssertDisable,
                              bool setTpulseDuration);

#endif /* INC_IMU_ICM42688_INTERRUPT_H_ */
