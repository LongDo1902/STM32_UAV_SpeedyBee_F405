/*
 * icm42688_data.h
 *
 *  Created on: Mar 14, 2026
 *      Author: dobao
 */

#ifndef INC_IMU_SENSORS_ICM42688_DATA_H_
#define INC_IMU_SENSORS_ICM42688_DATA_H_

#include "imu/core/icm42688_masks.h"
#include "imu/core/icm42688_registers.h"
#include "imu/core/icm42688_rw.h"
#include "imu/core/icm42688_types.h"

typedef struct
{
    float temp_c;
    float accel_g[3];
    float gyro_dps[3];
} ICM42688_Temp_Accel_Gyro_Scaled_t;

typedef struct
{
    float roll;
    float pitch;
    float yaw;
} ICM42688_Est_Angle_complement_t;

HAL_StatusTypeDef
ICM42688_Get_Temperature_C(ICM42688_Handle_t *handle, float *out_temp_c);

HAL_StatusTypeDef
ICM42688_Get_Accel_XYZ(ICM42688_Handle_t *handle, int16_t *buf);

HAL_StatusTypeDef
ICM42688_Get_Accel_G(ICM42688_Handle_t *handle, float g[3]);

HAL_StatusTypeDef
ICM42688_Get_Gyro_XYZ(ICM42688_Handle_t *handle, int16_t *buf);

HAL_StatusTypeDef
ICM42688_Get_Gyro_DPS(ICM42688_Handle_t *handle, float dps[3]);

HAL_StatusTypeDef
ICM42688_Get_Temp_Accel_Gyro_Raw(ICM42688_Handle_t *handle, int16_t *buf);

HAL_StatusTypeDef
ICM42688_Get_Temp_Accel_Gyro_Scaled(ICM42688_Handle_t                 *handle,
                                    ICM42688_Temp_Accel_Gyro_Scaled_t *sampleOut);

HAL_StatusTypeDef
ICM42688_Get_Est_Angle_Complement(ICM42688_Handle_t               *handle,
                                  ICM42688_Est_Angle_complement_t *attitudeOut, float dt_s);

#endif /* INC_IMU_SENSORS_ICM42688_DATA_H_ */
