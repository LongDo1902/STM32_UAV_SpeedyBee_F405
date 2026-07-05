/*
 * icm42688_data.c
 *
 *  Created on: Mar 14, 2026
 *      Author: dobao
 */
#include "imu/sensors/icm42688_data.h"
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include <stddef.h>

/* ==========================================================================================
 * 	TEMPERATURE DATA ONLY
 * ========================================================================================== */
bool
ICM42688_Get_Temperature_C(ICM42688_Handle_t *handle, float *outTempC)
{
    if (!handle || !outTempC)
        return false;

    if (handle->temp_config.temp_state == TEMP_DISABLE)
        return false;

    uint8_t _buf[2] = {0};
    bool    _status = ICM42688_ReadRegs(handle, ICM42688_UB0_TEMP_DATA1, _buf, 2);
    if (!_status)
        return false;

    int16_t _raw = 0;
    if (handle->intf_config.sensor_data_endian == SENSOR_DATA_BIG_ENDIAN) {
        _raw = (int16_t)(((uint16_t)_buf[0] << 8) | (uint16_t)_buf[1]);
    }
    else {
        _raw = (int16_t)(((uint16_t)_buf[1] << 8) | (uint16_t)_buf[0]);
    }

    *outTempC = (float)((_raw / 132.48f) + 25.0f);

    return true;
}



/* ==========================================================================================
 * 	ACCEL DATA ONLY
 * ========================================================================================== */
bool
ICM42688_Get_Accel_XYZ(ICM42688_Handle_t *handle, int16_t *buf)
{
    if (!handle || !buf)
        return false;

    if (handle->accel_config.accel_mode == ACCEL_OFF)
        return false;

    uint8_t _raw[6] = {0};
    bool    _status = ICM42688_ReadRegs(handle, ICM42688_UB0_ACCEL_DATA_X1, _raw, 6);
    if (!_status)
        return false;

    // Sensor data registers are 16-bit high/low pairs; INTF_CONFIG0 selects the byte order.
    if (handle->intf_config.sensor_data_endian == SENSOR_DATA_BIG_ENDIAN) {
        buf[0] = (int16_t)(((uint16_t)_raw[0] << 8) | (uint16_t)_raw[1]);
        buf[1] = (int16_t)(((uint16_t)_raw[2] << 8) | (uint16_t)_raw[3]);
        buf[2] = (int16_t)(((uint16_t)_raw[4] << 8) | (uint16_t)_raw[5]);
    }
    else {
        buf[0] = (int16_t)(((uint16_t)_raw[1] << 8) | (uint16_t)_raw[0]);
        buf[1] = (int16_t)(((uint16_t)_raw[3] << 8) | (uint16_t)_raw[2]);
        buf[2] = (int16_t)(((uint16_t)_raw[5] << 8) | (uint16_t)_raw[4]);
    }
    return true;
}



bool
ICM42688_Get_Accel_G(ICM42688_Handle_t *handle, float g[3])
{
    if (!handle || !g)
        return false;

    if (handle->accel_g_per_lsb <= 0.0f)
        return false;

    int16_t _raw[3] = {0};
    bool    _status = ICM42688_Get_Accel_XYZ(handle, _raw);
    if (!_status)
        return false;

    const float _s = handle->accel_g_per_lsb;
    g[0]           = (float)(_raw[0] * _s);
    g[1]           = (float)(_raw[1] * _s);
    g[2]           = (float)(_raw[2] * _s);

    return true;
}



/* ==========================================================================================
 * 	GYRO DATA ONLY
 * ========================================================================================== */
bool
ICM42688_Get_Gyro_XYZ(ICM42688_Handle_t *handle, int16_t *buf)
{
    if (!handle || !buf)
        return false;

    if (handle->gyro_config.gyro_mode == GYRO_OFF)
        return false;

    uint8_t _raw[6] = {0};
    bool    _status = ICM42688_ReadRegs(handle, ICM42688_UB0_GYRO_DATA_X1, _raw, 6);
    if (!_status)
        return false;

    // Sensor data registers are 16-bit high/low pairs; INTF_CONFIG0 selects the byte order.
    if (handle->intf_config.sensor_data_endian == SENSOR_DATA_BIG_ENDIAN) {

        buf[0] = (int16_t)(((uint16_t)_raw[0] << 8) | (uint16_t)_raw[1]);
        buf[1] = (int16_t)(((uint16_t)_raw[2] << 8) | (uint16_t)_raw[3]);
        buf[2] = (int16_t)(((uint16_t)_raw[4] << 8) | (uint16_t)_raw[5]);
    }
    else {
        buf[0] = (int16_t)(((uint16_t)_raw[1] << 8) | (uint16_t)_raw[0]);
        buf[1] = (int16_t)(((uint16_t)_raw[3] << 8) | (uint16_t)_raw[2]);
        buf[2] = (int16_t)(((uint16_t)_raw[5] << 8) | (uint16_t)_raw[4]);
    }
    return true;
}



bool
ICM42688_Get_Gyro_DPS(ICM42688_Handle_t *handle, float dps[3])
{
    if (!handle || !dps)
        return false;

    if (handle->gyro_dps_per_lsb <= 0.0f)
        return false;

    int16_t _raw[3] = {0};
    bool    _status = ICM42688_Get_Gyro_XYZ(handle, _raw);
    if (!_status)
        return false;

    // Convert gyro raw counts to dps using the cached full-scale scale factor.
    const float _s = handle->gyro_dps_per_lsb;
    dps[0]         = (float)(_raw[0] * _s);
    dps[1]         = (float)(_raw[1] * _s);
    dps[2]         = (float)(_raw[2] * _s);

    return true;
}



/* ==========================================================================================
 * 	TEMP ACCEL GYRO DATA IN ONE BURST READ
 * ========================================================================================== */
bool
ICM42688_Get_Temp_Accel_Gyro_Raw(ICM42688_Handle_t *handle, ICM42688_Raw_t *outRaw)
{
    if (!handle || !outRaw)
        return false;

    if ((handle->temp_config.temp_state == TEMP_DISABLE) || (handle->accel_config.accel_mode == ACCEL_OFF) ||
        (handle->gyro_config.gyro_mode == GYRO_OFF))
        return false;

    uint8_t _raw[14] = {0};
    bool    _status  = ICM42688_ReadRegs(handle, ICM42688_UB0_TEMP_DATA1, _raw, 14);
    if (!_status)
        return false;

    if (handle->intf_config.sensor_data_endian == SENSOR_DATA_BIG_ENDIAN) {
        // Get temperature raw
        outRaw->raw_temperature = (int16_t)((uint16_t)_raw[0] << 8) | (uint16_t)_raw[1];

        // Get accel raw
        outRaw->raw_accel[0] = (int16_t)((uint16_t)_raw[2] << 8) | (uint16_t)_raw[3];
        outRaw->raw_accel[1] = (int16_t)((uint16_t)_raw[4] << 8) | (uint16_t)_raw[5];
        outRaw->raw_accel[2] = (int16_t)((uint16_t)_raw[6] << 8) | (uint16_t)_raw[7];

        // Get gyro raw
        outRaw->raw_gyro[0] = (int16_t)((uint16_t)_raw[8] << 8) | (uint16_t)_raw[9];
        outRaw->raw_gyro[1] = (int16_t)((uint16_t)_raw[10] << 8) | (uint16_t)_raw[11];
        outRaw->raw_gyro[2] = (int16_t)((uint16_t)_raw[12] << 8) | (uint16_t)_raw[13];
    }
    else {
        // Get temperature raw
        outRaw->raw_temperature = (int16_t)((uint16_t)_raw[1] << 8) | (uint16_t)_raw[0];

        // Get accel raw
        outRaw->raw_accel[0] = (int16_t)((uint16_t)_raw[3] << 8) | (uint16_t)_raw[2];
        outRaw->raw_accel[1] = (int16_t)((uint16_t)_raw[5] << 8) | (uint16_t)_raw[4];
        outRaw->raw_accel[2] = (int16_t)((uint16_t)_raw[7] << 8) | (uint16_t)_raw[6];

        // Get gyro raw
        outRaw->raw_gyro[0] = (int16_t)((uint16_t)_raw[9] << 8) | (uint16_t)_raw[8];
        outRaw->raw_gyro[1] = (int16_t)((uint16_t)_raw[11] << 8) | (uint16_t)_raw[10];
        outRaw->raw_gyro[2] = (int16_t)((uint16_t)_raw[13] << 8) | (uint16_t)_raw[12];
    }

    return true;
}



bool
ICM42688_Get_Calibrate_Raw(ICM42688_Handle_t *handle, ICM42688_Offset_Raw_t *offsetCalibratedRaw, uint32_t samples)
{
    if (!handle || !offsetCalibratedRaw || (samples == 0U))
        return false;

    ICM42688_Raw_t _raw = {0};

    int32_t _sum_accel_x = {0}, _sum_accel_y = {0}, _sum_accel_z = {0};
    int32_t _sum_gyro_x = {0}, _sum_gyro_y = {0}, _sum_gyro_z = {0};

    for (uint32_t _i = 0U; _i < samples; _i++) {
        if (!ICM42688_Get_Temp_Accel_Gyro_Raw(handle, &_raw)) {
            return false;
        }

        _sum_accel_x += _raw.raw_accel[0];
        _sum_accel_y += _raw.raw_accel[1];
        _sum_accel_z += _raw.raw_accel[2];

        _sum_gyro_x += _raw.raw_gyro[0];
        _sum_gyro_y += _raw.raw_gyro[1];
        _sum_gyro_z += _raw.raw_gyro[2];

        // Calibration is not part of the real-time loop; spacing samples helps avoid reading repeated data.
        HAL_Delay(1U);
    }

    // Average raw offsets; subtract 1 g from Z so level gravity is not treated as sensor bias.
    offsetCalibratedRaw->offset_raw_accel[0] = (int32_t)(_sum_accel_x / (int32_t)samples);
    offsetCalibratedRaw->offset_raw_accel[1] = (int32_t)(_sum_accel_y / (int32_t)samples);
    offsetCalibratedRaw->offset_raw_accel[2] =
        (int32_t)(_sum_accel_z / (int32_t)samples) - (int32_t)(lroundf(handle->accel_lsb_per_g_dtsheet));

    offsetCalibratedRaw->offset_raw_gyro[0] = (int32_t)(_sum_gyro_x / (int32_t)samples);
    offsetCalibratedRaw->offset_raw_gyro[1] = (int32_t)(_sum_gyro_y / (int32_t)samples);
    offsetCalibratedRaw->offset_raw_gyro[2] = (int32_t)(_sum_gyro_z / (int32_t)samples);

    return true;
}



bool
ICM42688_Get_Temp_Accel_Gyro_Scaled(ICM42688_Handle_t *handle, const ICM42688_Offset_Raw_t *offsetRaw,
                                    ICM42688_Temp_Accel_Gyro_Scaled_t *sampleOut)
{
    if (!handle || !offsetRaw || !sampleOut)
        return false;

    if ((handle->gyro_dps_per_lsb <= 0.0f) || (handle->accel_g_per_lsb <= 0.0f))
        return false;

    ICM42688_Raw_t _raw    = {0};
    bool           _status = ICM42688_Get_Temp_Accel_Gyro_Raw(handle, &_raw);
    if (!_status)
        return false;

    const float _accel_s = handle->accel_g_per_lsb;
    const float _gyro_s  = handle->gyro_dps_per_lsb;

    // Temperature in C
    sampleOut->temp_c = (float)((_raw.raw_temperature / 132.48f) + 25.0f);

    // Accel in g
    sampleOut->accel_g[0] = (float)((_raw.raw_accel[0] - offsetRaw->offset_raw_accel[0]) * _accel_s);
    sampleOut->accel_g[1] = (float)((_raw.raw_accel[1] - offsetRaw->offset_raw_accel[1]) * _accel_s);
    sampleOut->accel_g[2] = (float)((_raw.raw_accel[2] - offsetRaw->offset_raw_accel[2]) * _accel_s);

    // Gyro in dps
    sampleOut->gyro_dps[0] = (float)((_raw.raw_gyro[0] - offsetRaw->offset_raw_gyro[0]) * _gyro_s);
    sampleOut->gyro_dps[1] = (float)((_raw.raw_gyro[1] - offsetRaw->offset_raw_gyro[1]) * _gyro_s);
    sampleOut->gyro_dps[2] = (float)((_raw.raw_gyro[2] - offsetRaw->offset_raw_gyro[2]) * _gyro_s);

    return true;
}



static float
ICM42688_GetMappedAxisValue(const float imuValue[3], ICM42688_Axis_t axis)
{
    switch (axis) {
        case AXIS_X:
            return imuValue[0];

        case AXIS_NEG_X:
            return -imuValue[0];

        case AXIS_Y:
            return imuValue[1];

        case AXIS_NEG_Y:
            return -imuValue[1];

        case AXIS_Z:
            return imuValue[2];

        case AXIS_NEG_Z:
            return -imuValue[2];

        default:
            return 0.0f;
    }
}



static const ICM42688_Remap_Axes_t imu_remap_orientation[IMU_ORIENT_COUNT] = {
    // Entries must stay in the same order as ICM42688_Orientation_t.
    // IMU Z+ aligned with body Z+.
    {AXIS_X, AXIS_Y, AXIS_Z},
    {AXIS_NEG_Y, AXIS_NEG_X, AXIS_Z},
    {AXIS_Y, AXIS_NEG_X, AXIS_Z},
    {AXIS_NEG_X, AXIS_NEG_Y, AXIS_Z},
    {AXIS_NEG_Y, AXIS_X, AXIS_Z},

    // IMU upside down: IMU Z- aligned with body Z+.
    {AXIS_X, AXIS_NEG_Y, AXIS_NEG_Z},
    {AXIS_NEG_Y, AXIS_NEG_X, AXIS_NEG_Z},
    {AXIS_NEG_X, AXIS_Y, AXIS_NEG_Z},
    {AXIS_Y, AXIS_X, AXIS_NEG_Z},
};



static bool
ICM42688_Remap_IMU_To_Body(ICM42688_Orientation_t orientation, const ICM42688_Temp_Accel_Gyro_Scaled_t *imuScaled,
                           ICM42688_Temp_Accel_Gyro_Scaled_t *bodyScaled)
{
    if (!imuScaled || !bodyScaled)
        return false;

    if ((uint8_t)orientation >= (uint8_t)IMU_ORIENT_COUNT)
        return false;

    const ICM42688_Remap_Axes_t *_remap = &imu_remap_orientation[orientation];

    bodyScaled->temp_c = imuScaled->temp_c;

    bodyScaled->accel_g[0] = ICM42688_GetMappedAxisValue(imuScaled->accel_g, _remap->body_x);
    bodyScaled->accel_g[1] = ICM42688_GetMappedAxisValue(imuScaled->accel_g, _remap->body_y);
    bodyScaled->accel_g[2] = ICM42688_GetMappedAxisValue(imuScaled->accel_g, _remap->body_z);

    bodyScaled->gyro_dps[0] = ICM42688_GetMappedAxisValue(imuScaled->gyro_dps, _remap->body_x);
    bodyScaled->gyro_dps[1] = ICM42688_GetMappedAxisValue(imuScaled->gyro_dps, _remap->body_y);
    bodyScaled->gyro_dps[2] = ICM42688_GetMappedAxisValue(imuScaled->gyro_dps, _remap->body_z);

    return true;
}



bool
ICM42688_Get_Est_Angle_Complement(ICM42688_Handle_t *handle, ICM42688_Orientation_t orientation,
                                  const ICM42688_Temp_Accel_Gyro_Scaled_t *inputImuScaled,
                                  ICM42688_Est_Angle_complement_t *attitudeOut, float dtS)
{
    if (!handle || !attitudeOut || !inputImuScaled)
        return false;

    ICM42688_Temp_Accel_Gyro_Scaled_t _body_scaled = {0};

    if (!ICM42688_Remap_IMU_To_Body(orientation, inputImuScaled, &_body_scaled))
        return false;

    float _accel_x = _body_scaled.accel_g[0];
    float _accel_y = _body_scaled.accel_g[1];
    float _accel_z = _body_scaled.accel_g[2];

    float _gyro_x = _body_scaled.gyro_dps[0];
    float _gyro_y = _body_scaled.gyro_dps[1];
    float _gyro_z = _body_scaled.gyro_dps[2];

    // Estimate roll and pitch from gravity, then blend them with integrated gyro rates.
    float _roll_acc  = atan2f(_accel_y, sqrt(_accel_z * _accel_z + _accel_x * _accel_x)) * 180.0f / M_PI;
    float _pitch_acc = atan2f(-_accel_x, sqrtf(_accel_y * _accel_y + _accel_z * _accel_z)) * 180.0f / M_PI;

    const float _alpha = 0.98f;

    attitudeOut->roll  = (_alpha * (attitudeOut->roll + _gyro_x * dtS)) + ((1.0f - _alpha) * _roll_acc);
    attitudeOut->pitch = (_alpha * (attitudeOut->pitch + _gyro_y * dtS)) + ((1.0f - _alpha) * _pitch_acc);
    // Yaw is gyro-only here because this driver has no magnetometer or external heading correction.
    attitudeOut->yaw   = attitudeOut->yaw + _gyro_z * dtS;

    return true;
}
