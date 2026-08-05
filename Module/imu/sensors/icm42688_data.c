/*
 * icm42688_data.c
 *
 *  Created on: Mar 14, 2026
 *      Author: dobaolong
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
ICM42688_Get_Temperature_C(ICM42688_Handle_t *pHandle, float *pOutTempC)
{
    if (!pHandle || !pOutTempC)
        return false;

    if (pHandle->temp_config.temp_state == TEMP_DISABLE)
        return false;

    uint8_t _buf[2] = {0};
    bool    _status = ICM42688_ReadRegs(pHandle, ICM42688_UB0_TEMP_DATA1, _buf, 2);
    if (!_status)
        return false;

    int16_t _raw = 0;
    if (pHandle->intf_config.sensor_data_endian == SENSOR_DATA_BIG_ENDIAN) {
        _raw = (int16_t)(((uint16_t)_buf[0] << 8) | (uint16_t)_buf[1]);
    }
    else {
        _raw = (int16_t)(((uint16_t)_buf[1] << 8) | (uint16_t)_buf[0]);
    }

    *pOutTempC = (float)((_raw / 132.48f) + 25.0f);

    return true;
}



/* ==========================================================================================
 * 	ACCEL DATA ONLY
 * ========================================================================================== */
bool
ICM42688_Get_Accel_XYZ(ICM42688_Handle_t *pHandle, int16_t *pBuf)
{
    if (!pHandle || !pBuf)
        return false;

    if (pHandle->accel_config.accel_mode == ACCEL_OFF)
        return false;

    uint8_t _raw[6] = {0};
    bool    _status = ICM42688_ReadRegs(pHandle, ICM42688_UB0_ACCEL_DATA_X1, _raw, 6);
    if (!_status)
        return false;

    // Sensor data registers are 16-bit high/low pairs; INTF_CONFIG0 selects the byte order.
    if (pHandle->intf_config.sensor_data_endian == SENSOR_DATA_BIG_ENDIAN) {
        pBuf[0] = (int16_t)(((uint16_t)_raw[0] << 8) | (uint16_t)_raw[1]);
        pBuf[1] = (int16_t)(((uint16_t)_raw[2] << 8) | (uint16_t)_raw[3]);
        pBuf[2] = (int16_t)(((uint16_t)_raw[4] << 8) | (uint16_t)_raw[5]);
    }
    else {
        pBuf[0] = (int16_t)(((uint16_t)_raw[1] << 8) | (uint16_t)_raw[0]);
        pBuf[1] = (int16_t)(((uint16_t)_raw[3] << 8) | (uint16_t)_raw[2]);
        pBuf[2] = (int16_t)(((uint16_t)_raw[5] << 8) | (uint16_t)_raw[4]);
    }
    return true;
}



bool
ICM42688_Get_Accel_G(ICM42688_Handle_t *pHandle, float pG[3])
{
    if (!pHandle || !pG)
        return false;

    if (pHandle->accel_g_per_lsb <= 0.0f)
        return false;

    int16_t _raw[3] = {0};
    bool    _status = ICM42688_Get_Accel_XYZ(pHandle, _raw);
    if (!_status)
        return false;

    const float _s = pHandle->accel_g_per_lsb;
    pG[0]          = (float)(_raw[0] * _s);
    pG[1]          = (float)(_raw[1] * _s);
    pG[2]          = (float)(_raw[2] * _s);

    return true;
}



/* ==========================================================================================
 * 	GYRO DATA ONLY
 * ========================================================================================== */
bool
ICM42688_Get_Gyro_XYZ(ICM42688_Handle_t *pHandle, int16_t *pBuf)
{
    if (!pHandle || !pBuf)
        return false;

    if (pHandle->gyro_config.gyro_mode == GYRO_OFF)
        return false;

    uint8_t _raw[6] = {0};
    bool    _status = ICM42688_ReadRegs(pHandle, ICM42688_UB0_GYRO_DATA_X1, _raw, 6);
    if (!_status)
        return false;

    // Sensor data registers are 16-bit high/low pairs; INTF_CONFIG0 selects the byte order.
    if (pHandle->intf_config.sensor_data_endian == SENSOR_DATA_BIG_ENDIAN) {

        pBuf[0] = (int16_t)(((uint16_t)_raw[0] << 8) | (uint16_t)_raw[1]);
        pBuf[1] = (int16_t)(((uint16_t)_raw[2] << 8) | (uint16_t)_raw[3]);
        pBuf[2] = (int16_t)(((uint16_t)_raw[4] << 8) | (uint16_t)_raw[5]);
    }
    else {
        pBuf[0] = (int16_t)(((uint16_t)_raw[1] << 8) | (uint16_t)_raw[0]);
        pBuf[1] = (int16_t)(((uint16_t)_raw[3] << 8) | (uint16_t)_raw[2]);
        pBuf[2] = (int16_t)(((uint16_t)_raw[5] << 8) | (uint16_t)_raw[4]);
    }
    return true;
}



bool
ICM42688_Get_Gyro_DPS(ICM42688_Handle_t *pHandle, float pDps[3])
{
    if (!pHandle || !pDps)
        return false;

    if (pHandle->gyro_dps_per_lsb <= 0.0f)
        return false;

    int16_t _raw[3] = {0};
    bool    _status = ICM42688_Get_Gyro_XYZ(pHandle, _raw);
    if (!_status)
        return false;

    // Convert gyro raw counts to dps using the cached full-scale scale factor.
    const float _s = pHandle->gyro_dps_per_lsb;
    pDps[0]        = (float)(_raw[0] * _s);
    pDps[1]        = (float)(_raw[1] * _s);
    pDps[2]        = (float)(_raw[2] * _s);

    return true;
}



/* ==========================================================================================
 * 	TEMP ACCEL GYRO DATA IN ONE BURST READ
 * ========================================================================================== */
bool
ICM42688_Get_Temp_Accel_Gyro_Raw(ICM42688_Handle_t *pHandle, ICM42688_Raw_t *pOutRaw)
{
    if (!pHandle || !pOutRaw)
        return false;

    if ((pHandle->temp_config.temp_state == TEMP_DISABLE) ||
        (pHandle->accel_config.accel_mode == ACCEL_OFF) || (pHandle->gyro_config.gyro_mode == GYRO_OFF))
        return false;

    uint8_t _raw[14] = {0};
    bool    _status  = ICM42688_ReadRegs(pHandle, ICM42688_UB0_TEMP_DATA1, _raw, 14);
    if (!_status)
        return false;

    if (pHandle->intf_config.sensor_data_endian == SENSOR_DATA_BIG_ENDIAN) {
        // Get temperature raw
        pOutRaw->raw_temperature = (int16_t)((uint16_t)_raw[0] << 8) | (uint16_t)_raw[1];

        // Get accel raw
        pOutRaw->raw_accel[0] = (int16_t)((uint16_t)_raw[2] << 8) | (uint16_t)_raw[3];
        pOutRaw->raw_accel[1] = (int16_t)((uint16_t)_raw[4] << 8) | (uint16_t)_raw[5];
        pOutRaw->raw_accel[2] = (int16_t)((uint16_t)_raw[6] << 8) | (uint16_t)_raw[7];

        // Get gyro raw
        pOutRaw->raw_gyro[0] = (int16_t)((uint16_t)_raw[8] << 8) | (uint16_t)_raw[9];
        pOutRaw->raw_gyro[1] = (int16_t)((uint16_t)_raw[10] << 8) | (uint16_t)_raw[11];
        pOutRaw->raw_gyro[2] = (int16_t)((uint16_t)_raw[12] << 8) | (uint16_t)_raw[13];
    }
    else {
        // Get temperature raw
        pOutRaw->raw_temperature = (int16_t)((uint16_t)_raw[1] << 8) | (uint16_t)_raw[0];

        // Get accel raw
        pOutRaw->raw_accel[0] = (int16_t)((uint16_t)_raw[3] << 8) | (uint16_t)_raw[2];
        pOutRaw->raw_accel[1] = (int16_t)((uint16_t)_raw[5] << 8) | (uint16_t)_raw[4];
        pOutRaw->raw_accel[2] = (int16_t)((uint16_t)_raw[7] << 8) | (uint16_t)_raw[6];

        // Get gyro raw
        pOutRaw->raw_gyro[0] = (int16_t)((uint16_t)_raw[9] << 8) | (uint16_t)_raw[8];
        pOutRaw->raw_gyro[1] = (int16_t)((uint16_t)_raw[11] << 8) | (uint16_t)_raw[10];
        pOutRaw->raw_gyro[2] = (int16_t)((uint16_t)_raw[13] << 8) | (uint16_t)_raw[12];
    }

    return true;
}



bool
ICM42688_Get_Calibrate_Raw(ICM42688_Handle_t *pHandle, ICM42688_Offset_Raw_t *pOffsetCalibratedRaw,
                           uint32_t samples)
{
    if (!pHandle || !pOffsetCalibratedRaw || (samples == 0U))
        return false;

    ICM42688_Raw_t _raw = {0};

    int32_t _sum_accel_x = {0}, _sum_accel_y = {0}, _sum_accel_z = {0};
    int32_t _sum_gyro_x = {0}, _sum_gyro_y = {0}, _sum_gyro_z = {0};

    for (uint32_t _i = 0U; _i < samples; _i++) {
        if (!ICM42688_Get_Temp_Accel_Gyro_Raw(pHandle, &_raw)) {
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
    pOffsetCalibratedRaw->offset_raw_accel[0] = (int32_t)(_sum_accel_x / (int32_t)samples);
    pOffsetCalibratedRaw->offset_raw_accel[1] = (int32_t)(_sum_accel_y / (int32_t)samples);
    pOffsetCalibratedRaw->offset_raw_accel[2] =
        (int32_t)(_sum_accel_z / (int32_t)samples) - (int32_t)(lroundf(pHandle->accel_lsb_per_g_dtsheet));

    pOffsetCalibratedRaw->offset_raw_gyro[0] = (int32_t)(_sum_gyro_x / (int32_t)samples);
    pOffsetCalibratedRaw->offset_raw_gyro[1] = (int32_t)(_sum_gyro_y / (int32_t)samples);
    pOffsetCalibratedRaw->offset_raw_gyro[2] = (int32_t)(_sum_gyro_z / (int32_t)samples);

    return true;
}



bool
ICM42688_Get_Temp_Accel_Gyro_Scaled(ICM42688_Handle_t *pHandle, const ICM42688_Offset_Raw_t *pOffsetRaw,
                                    ICM42688_Temp_Accel_Gyro_Scaled_t *pSampleOut)
{
    if (!pHandle || !pOffsetRaw || !pSampleOut)
        return false;

    if ((pHandle->gyro_dps_per_lsb <= 0.0f) || (pHandle->accel_g_per_lsb <= 0.0f))
        return false;

    ICM42688_Raw_t _raw    = {0};
    bool           _status = ICM42688_Get_Temp_Accel_Gyro_Raw(pHandle, &_raw);
    if (!_status)
        return false;

    const float _accel_s = pHandle->accel_g_per_lsb;
    const float _gyro_s  = pHandle->gyro_dps_per_lsb;

    // Temperature in C
    pSampleOut->temp_c = (float)((_raw.raw_temperature / 132.48f) + 25.0f);

    // Accel in g
    pSampleOut->accel_g[0] = (float)((_raw.raw_accel[0] - pOffsetRaw->offset_raw_accel[0]) * _accel_s);
    pSampleOut->accel_g[1] = (float)((_raw.raw_accel[1] - pOffsetRaw->offset_raw_accel[1]) * _accel_s);
    pSampleOut->accel_g[2] = (float)((_raw.raw_accel[2] - pOffsetRaw->offset_raw_accel[2]) * _accel_s);

    // Gyro in dps
    pSampleOut->gyro_dps[0] = (float)((_raw.raw_gyro[0] - pOffsetRaw->offset_raw_gyro[0]) * _gyro_s);
    pSampleOut->gyro_dps[1] = (float)((_raw.raw_gyro[1] - pOffsetRaw->offset_raw_gyro[1]) * _gyro_s);
    pSampleOut->gyro_dps[2] = (float)((_raw.raw_gyro[2] - pOffsetRaw->offset_raw_gyro[2]) * _gyro_s);

    return true;
}



static float
ICM42688_GetMappedAxisValue(const float pImuValue[3], ICM42688_Axis_t axis)
{
    switch (axis) {
        case AXIS_X:
            return pImuValue[0];

        case AXIS_NEG_X:
            return -pImuValue[0];

        case AXIS_Y:
            return pImuValue[1];

        case AXIS_NEG_Y:
            return -pImuValue[1];

        case AXIS_Z:
            return pImuValue[2];

        case AXIS_NEG_Z:
            return -pImuValue[2];

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
ICM42688_Remap_IMU_To_Body(ICM42688_Orientation_t                   orientation,
                           const ICM42688_Temp_Accel_Gyro_Scaled_t *pImuScaled,
                           ICM42688_Temp_Accel_Gyro_Scaled_t       *pBodyScaled)
{
    if (!pImuScaled || !pBodyScaled)
        return false;

    if ((uint8_t)orientation >= (uint8_t)IMU_ORIENT_COUNT)
        return false;

    const ICM42688_Remap_Axes_t *_remap = &imu_remap_orientation[orientation];

    pBodyScaled->temp_c = pImuScaled->temp_c;

    pBodyScaled->accel_g[0] = ICM42688_GetMappedAxisValue(pImuScaled->accel_g, _remap->body_x);
    pBodyScaled->accel_g[1] = ICM42688_GetMappedAxisValue(pImuScaled->accel_g, _remap->body_y);
    pBodyScaled->accel_g[2] = ICM42688_GetMappedAxisValue(pImuScaled->accel_g, _remap->body_z);

    pBodyScaled->gyro_dps[0] = ICM42688_GetMappedAxisValue(pImuScaled->gyro_dps, _remap->body_x);
    pBodyScaled->gyro_dps[1] = ICM42688_GetMappedAxisValue(pImuScaled->gyro_dps, _remap->body_y);
    pBodyScaled->gyro_dps[2] = ICM42688_GetMappedAxisValue(pImuScaled->gyro_dps, _remap->body_z);

    return true;
}



bool
ICM42688_Get_Est_Angle_Complement(ICM42688_Handle_t *pHandle, ICM42688_Orientation_t orientation,
                                  const ICM42688_Temp_Accel_Gyro_Scaled_t *pInputImuScaled,
                                  ICM42688_Est_Angle_complement_t *pAttitudeOut, float dtS)
{
    if (!pHandle || !pAttitudeOut || !pInputImuScaled)
        return false;

    ICM42688_Temp_Accel_Gyro_Scaled_t _body_scaled = {0};

    if (!ICM42688_Remap_IMU_To_Body(orientation, pInputImuScaled, &_body_scaled))
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

    pAttitudeOut->roll  = (_alpha * (pAttitudeOut->roll + _gyro_x * dtS)) + ((1.0f - _alpha) * _roll_acc);
    pAttitudeOut->pitch = (_alpha * (pAttitudeOut->pitch + _gyro_y * dtS)) + ((1.0f - _alpha) * _pitch_acc);
    // Yaw is gyro-only here because this driver has no magnetometer or external heading correction.
    pAttitudeOut->yaw = pAttitudeOut->yaw + _gyro_z * dtS;

    return true;
}
