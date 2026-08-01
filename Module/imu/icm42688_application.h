/*
 * icm42688_application.h
 *
 *  Created on: Mar 12, 2026
 *      Author: dobaolong
 */

#ifndef INC_IMU_ICM42688_DEVICE_H_
#define INC_IMU_ICM42688_DEVICE_H_

#include "imu/core/icm42688_masks.h"
#include "imu/core/icm42688_registers.h"
#include "imu/core/icm42688_rw.h"
#include "imu/core/icm42688_types.h"

#include "imu/features/icm42688_interface.h"
#include "imu/features/icm42688_interrupt.h"

#include "imu/features/icm42688_fifo.h"
#include "imu/sensors/icm42688_accel.h"
#include "imu/sensors/icm42688_data.h"
#include "imu/sensors/icm42688_gyro.h"
#include "imu/sensors/icm42688_temp.h"

#include <FreeRTOS.h>
#include <task.h>



#define ICM42688_WHO_AM_I_DEFAULT 0x47U

// Helper for init/config sequences: return false immediately when a step fails.
#define CHECK_FOR(expr)            \
    do {                           \
        _status = (expr);          \
        if (!_status)     \
            return false; \
    } while (0)

/**
 * @brief   Check whether the connected device reports the expected ICM42688 identity.
 *          The function reads WHO_AM_I, compares it with ICM42688_WHO_AM_I_DEFAULT, and updates the cached
 *          device-alive flag in the handle so later initialization code can reuse the result.
 * @param   pHandle  Pointer to the ICM42688 handle struct.
 * @return  true when WHO_AM_I matches the expected value, otherwise false.
 */
bool ICM42688_IsAlive(ICM42688_Handle_t *pHandle);

/**
 * @brief   Perform a software reset and restore the cached handle state to defaults.
 *          The function waits for INT_RESET_DONE before rebuilding the software-side register cache; callers
 *          must provide a working SPI interface because polling is performed synchronously.
 * @param   pHandle  Pointer to the ICM42688 handle struct.
 * @return  true when the reset completes successfully, otherwise false.
 */
bool ICM42688_SoftReset(ICM42688_Handle_t *pHandle);

/**
 * @brief   Initialize the ICM42688 interface, sensors, interrupts, and FIFO.
 *          Configuration is applied as a fail-fast sequence, and the handle is marked initialized only after
 *          every step succeeds. A failure can leave earlier hardware settings applied, so retry via reset.
 * @param   pHandle  Pointer to the ICM42688 handle struct.
 * @return  true when every initialization step succeeds, otherwise false.
 */
bool ICM42688_Init(ICM42688_Handle_t *pHandle);

#endif /* INC_IMU_ICM42688_DEVICE_H_ */
