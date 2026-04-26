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

    uint8_t           buf[2] = {0};
    HAL_StatusTypeDef status = ICM42688_ReadRegs(handle, ICM42688_UB0_TEMP_DATA1, buf, 2);
    if (status != HAL_OK)
        return ICM42688_ERROR;

    int16_t raw = 0;
    if (handle->intf_config.sensor_data_endian == SENSOR_DATA_BIG_ENDIAN) {
        raw = (int16_t)(((uint16_t)buf[0] << 8) | (uint16_t)buf[1]);
    }
    else {
        raw = (int16_t)(((uint16_t)buf[1] << 8) | (uint16_t)buf[0]);
    }

    *out_temp_c = (float)((raw / 132.48f) + 25.0f);

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

    uint8_t           raw[6] = {0};
    HAL_StatusTypeDef status = ICM42688_ReadRegs(handle, ICM42688_UB0_ACCEL_DATA_X1, raw, 6);
    if (status != HAL_OK)
        return ICM42688_ERROR;

    if (handle->intf_config.sensor_data_endian == SENSOR_DATA_BIG_ENDIAN) {
        buf[0] = (int16_t)(((uint16_t)raw[0] << 8) | (uint16_t)raw[1]); // Extract Accel X

        buf[1] = (int16_t)(((uint16_t)raw[2] << 8) | (uint16_t)raw[3]); // Extract Accel Y

        buf[2] = (int16_t)(((uint16_t)raw[4] << 8) | (uint16_t)raw[5]); // Extract Accel Z
    }
    else {
        buf[0] = (int16_t)(((uint16_t)raw[1] << 8) | (uint16_t)raw[0]); // Extract Accel X

        buf[1] = (int16_t)(((uint16_t)raw[3] << 8) | (uint16_t)raw[2]); // Extract Accel Y

        buf[2] = (int16_t)(((uint16_t)raw[5] << 8) | (uint16_t)raw[4]); // Extract Accel Z
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

    int16_t           raw[3] = {0};
    HAL_StatusTypeDef status = ICM42688_Get_Accel_XYZ(handle, raw);
    if (status != HAL_OK)
        return ICM42688_ERROR;

    const float s = handle->accel_g_per_lsb;
    g[0]          = (float)(raw[0] * s);
    g[1]          = (float)(raw[1] * s);
    g[2]          = (float)(raw[2] * s);

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

    uint8_t           raw[6] = {0};
    HAL_StatusTypeDef status = ICM42688_ReadRegs(handle, ICM42688_UB0_GYRO_DATA_X1, raw, 6);
    if (status != HAL_OK)
        return ICM42688_ERROR;

    if (handle->intf_config.sensor_data_endian == SENSOR_DATA_BIG_ENDIAN) {

        buf[0] = (int16_t)(((uint16_t)raw[0] << 8) | (uint16_t)raw[1]); // Extract Gyro X

        buf[1] = (int16_t)(((uint16_t)raw[2] << 8) | (uint16_t)raw[3]); // Extract Gyro Y

        buf[2] = (int16_t)(((uint16_t)raw[4] << 8) | (uint16_t)raw[5]); // Extract Gyro Z
    }
    else {
        buf[0] = (int16_t)(((uint16_t)raw[1] << 8) | (uint16_t)raw[0]); // Extract Gyro X

        buf[1] = (int16_t)(((uint16_t)raw[3] << 8) | (uint16_t)raw[2]); // Extract Gyro Y

        buf[2] = (int16_t)(((uint16_t)raw[5] << 8) | (uint16_t)raw[4]); // Extract Gyro Z
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

    int16_t           raw[3] = {0};
    HAL_StatusTypeDef status = ICM42688_Get_Gyro_XYZ(handle, raw);
    if (status != HAL_OK)
        return ICM42688_ERROR;

    // Extract gyro X, Y and Z dps
    const float s = handle->gyro_dps_per_lsb;
    dps[0]        = (float)(raw[0] * s);
    dps[1]        = (float)(raw[1] * s);
    dps[2]        = (float)(raw[2] * s);

    return ICM42688_OK;
}



/* ==========================================================================================
 * 	TEMP ACCEL GYRO DATA IN ONE BURST READ
 * ========================================================================================== */
ICM42688_Status_t
ICM42688_Get_Temp_Accel_Gyro_Raw(ICM42688_Handle_t *handle, ICM42688_Raw_t *outRaw)
{
    if (!handle || !outRaw)
        return ICM42688_ERROR;

    if ((handle->temp_config.temp_state == TEMP_DISABLE) ||
        (handle->accel_config.accel_mode == ACCEL_OFF) ||
        (handle->gyro_config.gyro_mode == GYRO_OFF))
        return ICM42688_ERROR;

    uint8_t           raw[14] = {0};
    HAL_StatusTypeDef status  = ICM42688_ReadRegs(handle, ICM42688_UB0_TEMP_DATA1, raw, 14);
    if (status != HAL_OK)
        return ICM42688_ERROR;

    if (handle->intf_config.sensor_data_endian == SENSOR_DATA_BIG_ENDIAN) {
        // Get temperature raw
        outRaw->raw_temperature = (int16_t)((uint16_t)raw[0] << 8) | (uint16_t)raw[1];

        // Get accel raw
        outRaw->raw_accel[0] = (int16_t)((uint16_t)raw[2] << 8) | (uint16_t)raw[3];
        outRaw->raw_accel[1] = (int16_t)((uint16_t)raw[4] << 8) | (uint16_t)raw[5];
        outRaw->raw_accel[2] = (int16_t)((uint16_t)raw[6] << 8) | (uint16_t)raw[7];

        // Get gyro raw
        outRaw->raw_gyro[0] = (int16_t)((uint16_t)raw[8] << 8) | (uint16_t)raw[9];
        outRaw->raw_gyro[1] = (int16_t)((uint16_t)raw[10] << 8) | (uint16_t)raw[11];
        outRaw->raw_gyro[2] = (int16_t)((uint16_t)raw[12] << 8) | (uint16_t)raw[13];
    }
    else {
        // Get temperature raw
        outRaw->raw_temperature = (int16_t)((uint16_t)raw[1] << 8) | (uint16_t)raw[0];

        // Get accel raw
        outRaw->raw_accel[0] = (int16_t)((uint16_t)raw[3] << 8) | (uint16_t)raw[2];
        outRaw->raw_accel[1] = (int16_t)((uint16_t)raw[5] << 8) | (uint16_t)raw[4];
        outRaw->raw_accel[2] = (int16_t)((uint16_t)raw[7] << 8) | (uint16_t)raw[6];

        // Get gyro raw
        outRaw->raw_gyro[0] = (int16_t)((uint16_t)raw[9] << 8) | (uint16_t)raw[8];
        outRaw->raw_gyro[1] = (int16_t)((uint16_t)raw[11] << 8) | (uint16_t)raw[10];
        outRaw->raw_gyro[2] = (int16_t)((uint16_t)raw[13] << 8) | (uint16_t)raw[12];
    }

    return ICM42688_OK;
}



HAL_StatusTypeDef
ICM42688_Get_Calibrate_Raw(ICM42688_Handle_t *handle, ICM42688_Offset_Raw_t *offsetCalibratedRaw,
                           uint32_t samples)
{
    if (!handle || !offsetCalibratedRaw)
        return HAL_ERROR;

    ICM42688_Raw_t raw = {0};

    int32_t sumAccelX = {0}, sumAccelY = {0}, sumAccelZ = {0};
    int32_t sumGyroX = {0}, sumGyroY = {0}, sumGyroZ = {0};

    for (size_t i = 0; i < samples; i++) {
        (void)ICM42688_Get_Temp_Accel_Gyro_Raw(handle, &raw);

        sumAccelX += raw.raw_accel[0]; // Accel X
        sumAccelY += raw.raw_accel[1]; // Accel Y
        sumAccelZ += raw.raw_accel[2]; // Accel Z

        sumGyroX += raw.raw_gyro[0]; // Gyro X
        sumGyroY += raw.raw_gyro[1]; // Gyro Y
        sumGyroZ += raw.raw_gyro[2]; // Gyro Z
    }

    // Save the offsets
    offsetCalibratedRaw->offset_raw_accel[0] = (int32_t)(sumAccelX / (int32_t)samples);
    offsetCalibratedRaw->offset_raw_accel[1] = (int32_t)(sumAccelY / (int32_t)samples);
    offsetCalibratedRaw->offset_raw_accel[2] = (int32_t)(sumAccelZ / (int32_t)samples) -
                                               (int32_t)(lroundf(handle->accel_lsb_per_g_dtsheet));

    offsetCalibratedRaw->offset_raw_gyro[0] = (int32_t)(sumGyroX / (int32_t)samples);
    offsetCalibratedRaw->offset_raw_gyro[1] = (int32_t)(sumGyroY / (int32_t)samples);
    offsetCalibratedRaw->offset_raw_gyro[2] = (int32_t)(sumGyroZ / (int32_t)samples);

    return HAL_OK;
}



ICM42688_Status_t
ICM42688_Get_Temp_Accel_Gyro_Scaled(ICM42688_Handle_t                 *handle,
                                    const ICM42688_Offset_Raw_t       *offsetRaw,
                                    ICM42688_Temp_Accel_Gyro_Scaled_t *sampleOut)
{
    if (!handle || !offsetRaw || !sampleOut)
        return ICM42688_ERROR;

    if ((handle->gyro_dps_per_lsb <= 0.0f) || (handle->accel_g_per_lsb <= 0.0f))
        return ICM42688_ERROR;

    ICM42688_Raw_t    raw    = {0};
    ICM42688_Status_t status = ICM42688_Get_Temp_Accel_Gyro_Raw(handle, &raw);
    if (status != HAL_OK)
        return ICM42688_ERROR;

    const float accel_s = handle->accel_g_per_lsb;
    const float gyro_s  = handle->gyro_dps_per_lsb;

    // Temperature in C
    sampleOut->temp_c = (float)((raw.raw_temperature / 132.48f) + 25.0f);

    // Accel in g
    sampleOut->accel_g[0] = (float)((raw.raw_accel[0] - offsetRaw->offset_raw_accel[0]) * accel_s);
    sampleOut->accel_g[1] = (float)((raw.raw_accel[1] - offsetRaw->offset_raw_accel[1]) * accel_s);
    sampleOut->accel_g[2] = (float)((raw.raw_accel[2] - offsetRaw->offset_raw_accel[2]) * accel_s);

    // Gyro in dps
    sampleOut->gyro_dps[0] = (float)((raw.raw_gyro[0] - offsetRaw->offset_raw_gyro[0]) * gyro_s);
    sampleOut->gyro_dps[1] = (float)((raw.raw_gyro[1] - offsetRaw->offset_raw_gyro[1]) * gyro_s);
    sampleOut->gyro_dps[2] = (float)((raw.raw_gyro[2] - offsetRaw->offset_raw_gyro[2]) * gyro_s);

    return ICM42688_OK;
}



HAL_StatusTypeDef
ICM42688_Get_Est_Angle_Complement(ICM42688_Handle_t                 *handle,
                                  ICM42688_Temp_Accel_Gyro_Scaled_t *scaledData,
                                  ICM42688_Est_Angle_complement_t *attitudeOut, float dt_s)
{
    if (!handle || !attitudeOut)
        return HAL_ERROR;

    float accel_x = scaledData->accel_g[0];
    float accel_y = scaledData->accel_g[1];
    float accel_z = scaledData->accel_g[2];

    float gyro_x = scaledData->gyro_dps[0];
    float gyro_y = scaledData->gyro_dps[1];
    float gyro_z = scaledData->gyro_dps[2];

    // Calculate estimated angle from accelerometer
    float roll_acc = atan2f(accel_y, sqrt(accel_z * accel_z + accel_x * accel_x)) * 180.0f / M_PI;
    float pitch_acc =
        atan2f(-accel_x, sqrtf(accel_y * accel_y + accel_z * accel_z)) * 180.0f / M_PI;

    const float alpha = 0.98f;

    attitudeOut->roll = (alpha * (attitudeOut->roll + gyro_x * dt_s)) + ((1.0f - alpha) * roll_acc);
    attitudeOut->pitch =
        (alpha * (attitudeOut->pitch + gyro_y * dt_s)) + ((1.0f - alpha) * pitch_acc);
    attitudeOut->yaw = attitudeOut->yaw + gyro_z * dt_s;

    return HAL_OK;
}