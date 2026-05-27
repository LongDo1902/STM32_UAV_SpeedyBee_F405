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
    int16_t raw_temperature;
    int16_t raw_accel[3];
    int16_t raw_gyro[3];

} ICM42688_Raw_t;


typedef struct
{
    int32_t offset_raw_accel[3];
    int32_t offset_raw_gyro[3];

} ICM42688_Offset_Raw_t;


typedef struct
{
    float temp_c;
    float accel_g[3];
    float gyro_dps[3];
} ICM42688_Temp_Accel_Gyro_Scaled_t;


typedef enum
{
    // IMU Z+ same as Body Z+
    IMU_ORIENT_X_Y_Z,
    IMU_ORIENT_NEGY_NEGX_Z,
    IMU_ORIENT_Y_NEG_X_Z,
    IMU_ORIENT_NEGX_NEGY_Z,
    IMU_ORIENT_NEGY_X_Z,

    // IMU is upside down, so Z- is same as Body Z+
    IMU_ORIENT_X_NEGY_NEGZ,
    IMU_ORIENT_NEGY_NEGX_NEGZ,
    IMU_ORIENT_NEGX_Y_NEGZ,
    IMU_ORIENT_Y_X_NEGZ,

    IMU_ORIENT_COUNT,
} ICM42688_Orientation_t;


typedef enum
{
    AXIS_X,
    AXIS_NEG_X,

    AXIS_Y,
    AXIS_NEG_Y,

    AXIS_Z,
    AXIS_NEG_Z
} ICM42688_Axis_t;


typedef struct
{
    ICM42688_Axis_t body_x;
    ICM42688_Axis_t body_y;
    ICM42688_Axis_t body_z;
} ICM42688_Remap_Axes_t;


typedef struct
{
    float roll;
    float pitch;
    float yaw;
} ICM42688_Est_Angle_complement_t;


bool
ICM42688_Get_Temperature_C(ICM42688_Handle_t *handle, float *out_temp_c);

bool
ICM42688_Get_Accel_XYZ(ICM42688_Handle_t *handle, int16_t *buf);

bool
ICM42688_Get_Accel_G(ICM42688_Handle_t *handle, float g[3]);

bool
ICM42688_Get_Gyro_XYZ(ICM42688_Handle_t *handle, int16_t *buf);

bool
ICM42688_Get_Gyro_DPS(ICM42688_Handle_t *handle, float dps[3]);

bool
ICM42688_Get_Temp_Accel_Gyro_Raw(ICM42688_Handle_t *handle, ICM42688_Raw_t *out_raw);

bool
ICM42688_Get_Calibrate_Raw(ICM42688_Handle_t *handle, ICM42688_Offset_Raw_t *offset_calibrated_raw,
                           uint32_t samples);

bool
ICM42688_Get_Temp_Accel_Gyro_Scaled(ICM42688_Handle_t                 *handle,
                                    const ICM42688_Offset_Raw_t       *offset_raw,
                                    ICM42688_Temp_Accel_Gyro_Scaled_t *sample_out);

bool
ICM42688_Get_Est_Angle_Complement(ICM42688_Handle_t *handle, ICM42688_Orientation_t orientation,
                                  const ICM42688_Temp_Accel_Gyro_Scaled_t *input_imu_scaled,
                                  ICM42688_Est_Angle_complement_t *attitude_out, float dt_s);

#endif /* INC_IMU_SENSORS_ICM42688_DATA_H_ */
