/*
 * icm42688_data.c
 *
 *  Created on: Mar 14, 2026
 *      Author: dobao
 */
#include "imu/sensors/icm42688_data.h"
#include <math.h>

/* ==========================================================================================
 * 	TEMPERATURE DATA ONLY
 * ========================================================================================== */
ICM42688_Status_t
ICM42688_Get_Temperature_C(ICM42688_Handle_t *handle, float *out_temp_c)
{
    if (!handle || !out_temp_c)
        return ICM42688_ERROR;

    if (handle->temp_config.temp_state == TEMP_DISABLE)
        return ICM42688_ERROR;

    uint8_t           _buf[2] = {0};
    HAL_StatusTypeDef _status = ICM42688_ReadRegs(handle, ICM42688_UB0_TEMP_DATA1, _buf, 2);
    if (_status != HAL_OK)
        return ICM42688_ERROR;

    int16_t _raw = 0;
    if (handle->intf_config.sensor_data_endian == SENSOR_DATA_BIG_ENDIAN) {
        _raw = (int16_t)(((uint16_t)_buf[0] << 8) | (uint16_t)_buf[1]);
    }
    else {
        _raw = (int16_t)(((uint16_t)_buf[1] << 8) | (uint16_t)_buf[0]);
    }

    *out_temp_c = (float)((_raw / 132.48f) + 25.0f);

    return ICM42688_OK;
}



/* ==========================================================================================
 * 	ACCEL DATA ONLY
 * ========================================================================================== */
ICM42688_Status_t
ICM42688_Get_Accel_XYZ(ICM42688_Handle_t *handle, int16_t *buf)
{
    if (!handle || !buf)
        return ICM42688_ERROR;

    if (handle->accel_config.accel_mode == ACCEL_OFF)
        return ICM42688_ERROR;

    uint8_t           _raw[6] = {0};
    HAL_StatusTypeDef _status = ICM42688_ReadRegs(handle, ICM42688_UB0_ACCEL_DATA_X1, _raw, 6);
    if (_status != HAL_OK)
        return ICM42688_ERROR;

    if (handle->intf_config.sensor_data_endian == SENSOR_DATA_BIG_ENDIAN) {
        buf[0] = (int16_t)(((uint16_t)_raw[0] << 8) | (uint16_t)_raw[1]); // Extract Accel X
        buf[1] = (int16_t)(((uint16_t)_raw[2] << 8) | (uint16_t)_raw[3]); // Extract Accel Y
        buf[2] = (int16_t)(((uint16_t)_raw[4] << 8) | (uint16_t)_raw[5]); // Extract Accel Z
    }
    else {
        buf[0] = (int16_t)(((uint16_t)_raw[1] << 8) | (uint16_t)_raw[0]); // Extract Accel X
        buf[1] = (int16_t)(((uint16_t)_raw[3] << 8) | (uint16_t)_raw[2]); // Extract Accel Y
        buf[2] = (int16_t)(((uint16_t)_raw[5] << 8) | (uint16_t)_raw[4]); // Extract Accel Z
    }
    return ICM42688_OK;
}



ICM42688_Status_t
ICM42688_Get_Accel_G(ICM42688_Handle_t *handle, float g[3])
{
    if (!handle || !g)
        return ICM42688_ERROR;

    if (handle->accel_g_per_lsb <= 0.0f)
        return ICM42688_ERROR;

    int16_t           _raw[3] = {0};
    HAL_StatusTypeDef _status = ICM42688_Get_Accel_XYZ(handle, _raw);
    if (_status != HAL_OK)
        return ICM42688_ERROR;

    const float _s = handle->accel_g_per_lsb;
    g[0]           = (float)(_raw[0] * _s);
    g[1]           = (float)(_raw[1] * _s);
    g[2]           = (float)(_raw[2] * _s);

    return ICM42688_OK;
}



/* ==========================================================================================
 * 	GYRO DATA ONLY
 * ========================================================================================== */
ICM42688_Status_t
ICM42688_Get_Gyro_XYZ(ICM42688_Handle_t *handle, int16_t *buf)
{
    if (!handle || !buf)
        return ICM42688_ERROR;

    if (handle->gyro_config.gyro_mode == GYRO_OFF)
        return ICM42688_ERROR;

    uint8_t           _raw[6] = {0};
    HAL_StatusTypeDef _status = ICM42688_ReadRegs(handle, ICM42688_UB0_GYRO_DATA_X1, _raw, 6);
    if (_status != HAL_OK)
        return ICM42688_ERROR;

    if (handle->intf_config.sensor_data_endian == SENSOR_DATA_BIG_ENDIAN) {

        buf[0] = (int16_t)(((uint16_t)_raw[0] << 8) | (uint16_t)_raw[1]); // Extract Gyro X
        buf[1] = (int16_t)(((uint16_t)_raw[2] << 8) | (uint16_t)_raw[3]); // Extract Gyro Y
        buf[2] = (int16_t)(((uint16_t)_raw[4] << 8) | (uint16_t)_raw[5]); // Extract Gyro Z
    }
    else {
        buf[0] = (int16_t)(((uint16_t)_raw[1] << 8) | (uint16_t)_raw[0]); // Extract Gyro X
        buf[1] = (int16_t)(((uint16_t)_raw[3] << 8) | (uint16_t)_raw[2]); // Extract Gyro Y
        buf[2] = (int16_t)(((uint16_t)_raw[5] << 8) | (uint16_t)_raw[4]); // Extract Gyro Z
    }
    return ICM42688_OK;
}



ICM42688_Status_t
ICM42688_Get_Gyro_DPS(ICM42688_Handle_t *handle, float dps[3])
{
    if (!handle || !dps)
        return ICM42688_ERROR;

    if (handle->gyro_dps_per_lsb <= 0.0f)
        return ICM42688_ERROR;

    int16_t           _raw[3] = {0};
    HAL_StatusTypeDef _status = ICM42688_Get_Gyro_XYZ(handle, _raw);
    if (_status != HAL_OK)
        return ICM42688_ERROR;

    // Extract gyro X, Y and Z dps
    const float _s = handle->gyro_dps_per_lsb;
    dps[0]         = (float)(_raw[0] * _s);
    dps[1]         = (float)(_raw[1] * _s);
    dps[2]         = (float)(_raw[2] * _s);

    return ICM42688_OK;
}



/* ==========================================================================================
 * 	TEMP ACCEL GYRO DATA IN ONE BURST READ
 * ========================================================================================== */
ICM42688_Status_t
ICM42688_Get_Temp_Accel_Gyro_Raw(ICM42688_Handle_t *handle, ICM42688_Raw_t *out_raw)
{
    if (!handle || !out_raw)
        return ICM42688_ERROR;

    if ((handle->temp_config.temp_state == TEMP_DISABLE) ||
        (handle->accel_config.accel_mode == ACCEL_OFF) ||
        (handle->gyro_config.gyro_mode == GYRO_OFF))
        return ICM42688_ERROR;

    uint8_t           _raw[14] = {0};
    HAL_StatusTypeDef _status  = ICM42688_ReadRegs(handle, ICM42688_UB0_TEMP_DATA1, _raw, 14);
    if (_status != HAL_OK)
        return ICM42688_ERROR;

    if (handle->intf_config.sensor_data_endian == SENSOR_DATA_BIG_ENDIAN) {
        // Get temperature raw
        out_raw->raw_temperature = (int16_t)((uint16_t)_raw[0] << 8) | (uint16_t)_raw[1];

        // Get accel raw
        out_raw->raw_accel[0] = (int16_t)((uint16_t)_raw[2] << 8) | (uint16_t)_raw[3];
        out_raw->raw_accel[1] = (int16_t)((uint16_t)_raw[4] << 8) | (uint16_t)_raw[5];
        out_raw->raw_accel[2] = (int16_t)((uint16_t)_raw[6] << 8) | (uint16_t)_raw[7];

        // Get gyro raw
        out_raw->raw_gyro[0] = (int16_t)((uint16_t)_raw[8] << 8) | (uint16_t)_raw[9];
        out_raw->raw_gyro[1] = (int16_t)((uint16_t)_raw[10] << 8) | (uint16_t)_raw[11];
        out_raw->raw_gyro[2] = (int16_t)((uint16_t)_raw[12] << 8) | (uint16_t)_raw[13];
    }
    else {
        // Get temperature raw
        out_raw->raw_temperature = (int16_t)((uint16_t)_raw[1] << 8) | (uint16_t)_raw[0];

        // Get accel raw
        out_raw->raw_accel[0] = (int16_t)((uint16_t)_raw[3] << 8) | (uint16_t)_raw[2];
        out_raw->raw_accel[1] = (int16_t)((uint16_t)_raw[5] << 8) | (uint16_t)_raw[4];
        out_raw->raw_accel[2] = (int16_t)((uint16_t)_raw[7] << 8) | (uint16_t)_raw[6];

        // Get gyro raw
        out_raw->raw_gyro[0] = (int16_t)((uint16_t)_raw[9] << 8) | (uint16_t)_raw[8];
        out_raw->raw_gyro[1] = (int16_t)((uint16_t)_raw[11] << 8) | (uint16_t)_raw[10];
        out_raw->raw_gyro[2] = (int16_t)((uint16_t)_raw[13] << 8) | (uint16_t)_raw[12];
    }

    return ICM42688_OK;
}



HAL_StatusTypeDef
ICM42688_Get_Calibrate_Raw(ICM42688_Handle_t *handle, ICM42688_Offset_Raw_t *offset_calibrated_raw,
                           uint32_t samples)
{
    if (!handle || !offset_calibrated_raw || (samples == 0U))
        return HAL_ERROR;

    ICM42688_Raw_t _raw = {0};

    int32_t sum_accel_x = {0}, sum_accel_y = {0}, sum_accel_z = {0};
    int32_t sum_gyro_x = {0}, sum_gyro_y = {0}, sum_gyro_z = {0};

    for (size_t i = 0; i < samples; i++) {
        (void)ICM42688_Get_Temp_Accel_Gyro_Raw(handle, &_raw);

        sum_accel_x += _raw.raw_accel[0]; // Accel X
        sum_accel_y += _raw.raw_accel[1]; // Accel Y
        sum_accel_z += _raw.raw_accel[2]; // Accel Z

        sum_gyro_x += _raw.raw_gyro[0]; // Gyro X
        sum_gyro_y += _raw.raw_gyro[1]; // Gyro Y
        sum_gyro_z += _raw.raw_gyro[2]; // Gyro Z
    }

    // Save the offsets
    offset_calibrated_raw->offset_raw_accel[0] = (int32_t)(sum_accel_x / (int32_t)samples);
    offset_calibrated_raw->offset_raw_accel[1] = (int32_t)(sum_accel_y / (int32_t)samples);
    offset_calibrated_raw->offset_raw_accel[2] =
        (int32_t)(sum_accel_z / (int32_t)samples) -
        (int32_t)(lroundf(handle->accel_lsb_per_g_dtsheet));

    offset_calibrated_raw->offset_raw_gyro[0] = (int32_t)(sum_gyro_x / (int32_t)samples);
    offset_calibrated_raw->offset_raw_gyro[1] = (int32_t)(sum_gyro_y / (int32_t)samples);
    offset_calibrated_raw->offset_raw_gyro[2] = (int32_t)(sum_gyro_z / (int32_t)samples);

    return HAL_OK;
}



ICM42688_Status_t
ICM42688_Get_Temp_Accel_Gyro_Scaled(ICM42688_Handle_t                 *handle,
                                    const ICM42688_Offset_Raw_t       *offset_raw,
                                    ICM42688_Temp_Accel_Gyro_Scaled_t *sample_out)
{
    if (!handle || !offset_raw || !sample_out)
        return ICM42688_ERROR;

    if ((handle->gyro_dps_per_lsb <= 0.0f) || (handle->accel_g_per_lsb <= 0.0f))
        return ICM42688_ERROR;

    ICM42688_Raw_t    _raw    = {0};
    ICM42688_Status_t _status = ICM42688_Get_Temp_Accel_Gyro_Raw(handle, &_raw);
    if (_status != HAL_OK)
        return ICM42688_ERROR;

    const float _accel_s = handle->accel_g_per_lsb;
    const float _gyro_s  = handle->gyro_dps_per_lsb;

    // Temperature in C
    sample_out->temp_c = (float)((_raw.raw_temperature / 132.48f) + 25.0f);

    // Accel in g
    sample_out->accel_g[0] =
        (float)((_raw.raw_accel[0] - offset_raw->offset_raw_accel[0]) * _accel_s);
    sample_out->accel_g[1] =
        (float)((_raw.raw_accel[1] - offset_raw->offset_raw_accel[1]) * _accel_s);
    sample_out->accel_g[2] =
        (float)((_raw.raw_accel[2] - offset_raw->offset_raw_accel[2]) * _accel_s);

    // Gyro in dps
    sample_out->gyro_dps[0] =
        (float)((_raw.raw_gyro[0] - offset_raw->offset_raw_gyro[0]) * _gyro_s);
    sample_out->gyro_dps[1] =
        (float)((_raw.raw_gyro[1] - offset_raw->offset_raw_gyro[1]) * _gyro_s);
    sample_out->gyro_dps[2] =
        (float)((_raw.raw_gyro[2] - offset_raw->offset_raw_gyro[2]) * _gyro_s);

    return ICM42688_OK;
}



static float
ICM42688_GetMappedAxisValue(const float imu_value[3], ICM42688_Axis_t axis)
{
    switch (axis) {
        case AXIS_X:
            return imu_value[0];

        case AXIS_NEG_X:
            return -imu_value[0];

        case AXIS_Y:
            return imu_value[1];

        case AXIS_NEG_Y:
            return -imu_value[1];

        case AXIS_Z:
            return imu_value[2];

        case AXIS_NEG_Z:
            return -imu_value[2];

        default:
            return 0.0f;
    }
}



static const ICM42688_Remap_Axes_t imu_remap_orientation[IMU_ORIENT_COUNT] = {
    // 4 common cases in drone application
    // IMU Z+ same as body's Z+
    {AXIS_X, AXIS_Y, AXIS_Z},
    {AXIS_Y, AXIS_NEG_X, AXIS_Z},
    {AXIS_NEG_X, AXIS_NEG_Y, AXIS_Z},
    {AXIS_NEG_Y, AXIS_X, AXIS_Z},

    // IMU is upside down, so Z- is same as body's Z+
    {AXIS_X, AXIS_NEG_Y, AXIS_NEG_Z},
    {AXIS_NEG_Y, AXIS_NEG_X, AXIS_NEG_Z},
    {AXIS_NEG_X, AXIS_Y, AXIS_NEG_Z},
    {AXIS_Y, AXIS_X, AXIS_NEG_Z},
};



bool
ICM42688_Remap_IMU_To_Body(ICM42688_Orientation_t                   orientation,
                           const ICM42688_Temp_Accel_Gyro_Scaled_t *imu_scaled,
                           ICM42688_Temp_Accel_Gyro_Scaled_t       *body_scaled)
{
    if (!imu_scaled || !body_scaled)
        return false;

    const ICM42688_Remap_Axes_t *remap = &imu_remap_orientation[orientation];

    body_scaled->temp_c = imu_scaled->temp_c;

    body_scaled->accel_g[0] = ICM42688_GetMappedAxisValue(imu_scaled->accel_g, remap->body_x);
    body_scaled->accel_g[1] = ICM42688_GetMappedAxisValue(imu_scaled->accel_g, remap->body_y);
    body_scaled->accel_g[2] = ICM42688_GetMappedAxisValue(imu_scaled->accel_g, remap->body_z);

    body_scaled->gyro_dps[0] = ICM42688_GetMappedAxisValue(imu_scaled->gyro_dps, remap->body_x);
    body_scaled->gyro_dps[1] = ICM42688_GetMappedAxisValue(imu_scaled->gyro_dps, remap->body_y);
    body_scaled->gyro_dps[2] = ICM42688_GetMappedAxisValue(imu_scaled->gyro_dps, remap->body_z);

    return true;
}



HAL_StatusTypeDef
ICM42688_Get_Est_Angle_Complement(ICM42688_Handle_t                       *handle,
                                  const ICM42688_Temp_Accel_Gyro_Scaled_t *scaled_data,
                                  ICM42688_Est_Angle_complement_t *attitude_out, float dt_s)
{
    if (!handle || !attitude_out || !scaled_data)
        return HAL_ERROR;

    float _accel_x = scaled_data->accel_g[0];
    float _accel_y = scaled_data->accel_g[1];
    float _accel_z = scaled_data->accel_g[2];

    float _gyro_x = scaled_data->gyro_dps[0];
    float _gyro_y = scaled_data->gyro_dps[1];
    float _gyro_z = scaled_data->gyro_dps[2];

    // Calculate estimated angle from accelerometer
    float _roll_acc =
        atan2f(_accel_y, sqrt(_accel_z * _accel_z + _accel_x * _accel_x)) * 180.0f / M_PI;
    float _pitch_acc =
        atan2f(-_accel_x, sqrtf(_accel_y * _accel_y + _accel_z * _accel_z)) * 180.0f / M_PI;

    const float _alpha = 0.98f;

    attitude_out->roll =
        (_alpha * (attitude_out->roll + _gyro_x * dt_s)) + ((1.0f - _alpha) * _roll_acc);
    attitude_out->pitch =
        (_alpha * (attitude_out->pitch + _gyro_y * dt_s)) + ((1.0f - _alpha) * _pitch_acc);
    attitude_out->yaw = attitude_out->yaw + _gyro_z * dt_s;

    return HAL_OK;
}