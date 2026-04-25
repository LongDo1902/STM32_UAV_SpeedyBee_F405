/*
 * icm42688_device.h
 *
 *  Created on: Mar 12, 2026
 *      Author: dobao
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

#define ICM42688_WHO_AM_I_DEFAULT	0x47U

#define CHECK_FOR(expr)         \
do{                             \
    status = (expr);            \
    if(status != HAL_OK)        \
        return ICM42688_ERROR;  \
}while(0)

ICM42688_Status_t
ICM42688_IsAlive(ICM42688_Handle_t *handle);

ICM42688_Status_t
ICM42688_SoftReset(ICM42688_Handle_t *handle);

ICM42688_Status_t
ICM42688_Init(ICM42688_Handle_t *handle);

#endif /* INC_IMU_ICM42688_DEVICE_H_ */
