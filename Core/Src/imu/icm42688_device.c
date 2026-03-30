/*
 * icm42688_device.c
 *
 *  Created on: Mar 12, 2026
 *      Author: dobao
 */

#include "imu/icm42688_device.h"

/*=============================================================================
 *	IDENTITY / RESET /
 *============================================================================ */
static inline HAL_StatusTypeDef
ICM42688_Get_WhoAmI(ICM42688_Handle_t *handle, uint8_t *who_val)
{
    if (!handle || !who_val)
        return HAL_ERROR;
    return ICM42688_ReadReg(handle, ICM42688_UB0_WHO_AM_I, who_val);
}



HAL_StatusTypeDef
ICM42688_IsAlive(ICM42688_Handle_t *handle)
{
    if (!handle)
        return HAL_ERROR;

    uint8_t           who    = 0U;
    HAL_StatusTypeDef status = ICM42688_Get_WhoAmI(handle, &who);

    if (status != HAL_OK) {
        handle->is_icm42688_alive = false;
        return status;
    }

    if (who == ICM42688_WHO_AM_I_DEFAULT) {
        handle->is_icm42688_alive = true;
        return HAL_OK;
    }

    handle->is_icm42688_alive = false;
    return HAL_ERROR;
}



HAL_StatusTypeDef
ICM42688_SoftReset(ICM42688_Handle_t *handle)
{
    if (!handle)
        return HAL_ERROR;

    HAL_StatusTypeDef status = ICM42688_Update_Reg_Bits(handle, ICM42688_UB0_DEVICE_CONF,
                                                        ICM42688_DEVICE_CONFIG_SOFT_RESET_Msk,
                                                        ICM42688_DEVICE_CONFIG_SOFT_RESET_Msk);
    if (status != HAL_OK)
        return status;

    HAL_Delay(5);

    // After reset, set every flag to a default/known state
    handle->is_reset              = true;
    handle->is_initialized        = false;
    handle->is_icm42688_alive     = false;
    handle->cached.last_tag_valid = false;

    handle->gyro_dps_per_lsb         = 0.0f;
    handle->gyro_lsb_per_dps_dtsheet = 0.0f;

    handle->accel_g_per_lsb         = 0.0f;
    handle->accel_lsb_per_g_dtsheet = 0.0f;

    handle->cached.last_tag.temp_c      = 0.0f;
    handle->cached.last_tag.accel_g[0]  = 0.0f;
    handle->cached.last_tag.accel_g[1]  = 0.0f;
    handle->cached.last_tag.accel_g[2]  = 0.0f;
    handle->cached.last_tag.gyro_dps[0] = 0.0f;
    handle->cached.last_tag.gyro_dps[1] = 0.0f;
    handle->cached.last_tag.gyro_dps[2] = 0.0f;

    handle->temp_config.temp_state = TEMP_ENABLE;

    handle->gyro_config.gyro_odr        = GYRO_ODR_1KHz;
    handle->gyro_config.gyro_fsr        = GYRO_FSR_2000dps;
    handle->gyro_config.gyro_notch      = GYRO_NOTCHBW_680Hz;
    handle->gyro_config.gyro_filt_order = GYRO_SECOND_ORDER;
    handle->gyro_config.gyro_mode       = GYRO_OFF;
    handle->gyro_config.gyro_uifilt_bw  = BW_400Hz_ODR_DIV_4;

    handle->accel_config.accel_odr        = ACCEL_ODR_1KHz;
    handle->accel_config.accel_fsr        = ACCEL_FSR_16g;
    handle->accel_config.accel_filt_order = ACCEL_SECOND_ORDER;
    handle->accel_config.accel_mode       = ACCEL_OFF;
    handle->accel_config.accel_uifilt_bw  = BW_400Hz_ODR_DIV_4;

    handle->int1_config.int1_polarity = INT_ACTIVE_LOW;
    handle->int1_config.int1_drive    = INT_OPEN_DRAIN;
    handle->int1_config.int1_mode     = INT_PULSED;

    handle->int2_config.int2_polarity = INT_ACTIVE_LOW;
    handle->int2_config.int2_drive    = INT_OPEN_DRAIN;
    handle->int2_config.int2_mode     = INT_PULSED;

    handle->intf_config.ui_sifs_config     = UI_SIFS_RESERVED;
    handle->intf_config.sensor_data_endian = SENSOR_DATA_BIG_ENDIAN;

    handle->fifo_config.fifo_mode         = BYPASS;
    handle->fifo_config.fifo_count_endian = FIFO_COUNT_BIG_ENDIAN;
    handle->fifo_config.fifo_count_rec    = FIFO_COUNT_IN_BYTE;

    handle->fifo_config.fifo_gyro_state  = FIFO_GAT_ENABLE;
    handle->fifo_config.fifo_accel_state = FIFO_GAT_ENABLE;
    handle->fifo_config.fifo_temp_state  = FIFO_GAT_ENABLE;

    handle->fifo_config.fifo_wm_mode            = FIFO_WM_GREATER_THS_ONESHOT;
    handle->fifo_config.fifo_hires_state        = FIFO_HIRES_DISABLE;
    handle->fifo_config.fifo_partial_read_state = FIFO_PARTIAL_READ_DISABLE;

    return status;
}



HAL_StatusTypeDef
ICM42688_Init(ICM42688_Handle_t *handle)
{
    if (!handle)
        return HAL_ERROR;

    HAL_StatusTypeDef status = HAL_OK;

    CHECK_FOR(ICM42688_IsAlive(handle));

    // Reset and gating
    CHECK_FOR(ICM42688_Set_Int1_ResetDone_Enable(handle, true));
    CHECK_FOR(ICM42688_SoftReset(handle));

    uint8_t intStatus     = 0U;
    bool    resetDoneFlag = false;
    do {
        CHECK_FOR(ICM42688_Get_Int_Status(handle, &intStatus));
    } while (!ICM42688_Int_Status_Has(intStatus, INT_RESET_DONE));

    // Interface configuration
    CHECK_FOR(ICM42688_Set_SPI_Mode(handle, SPI_MODE_0_3));
    CHECK_FOR(ICM42688_Set_SPI_SlewRate(handle, SPI_SR_2NS));
    CHECK_FOR(ICM42688_Set_UI_SIFS_Conf(handle, UI_SIFS_DISABLE_I2C));
    CHECK_FOR(ICM42688_Set_Sensor_Data_Endian(handle, SENSOR_DATA_BIG_ENDIAN));

    // Accel configuring
    CHECK_FOR(ICM42688_Set_AccelConfig(handle, ACCEL_LOW_NOISE, ACCEL_ODR_8KHz, ACCEL_FSR_8g));
    CHECK_FOR(ICM42688_Set_Accel_UIFilt_BW(handle, BW_ODR_DIV_2));
    CHECK_FOR(ICM42688_Set_Accel_UIFilt_Order(handle, ACCEL_FIRST_ORDER));
    CHECK_FOR(ICM42688_Set_Accel_Anti_Alias_Filt(handle, ENABLE_AAF));

    // Gyro configuring
    CHECK_FOR(ICM42688_Set_GyroConfig(handle, GYRO_LOW_NOISE, GYRO_ODR_8KHz, GYRO_FSR_2000dps));
    CHECK_FOR(ICM42688_Set_Gyro_UIFilt_BW(handle, BW_ODR_DIV_2));
    CHECK_FOR(ICM42688_Set_Gyro_UIFilt_Order(handle, GYRO_FIRST_ORDER));
    CHECK_FOR(ICM42688_Set_Gyro_Anti_Alias_Filt(handle, ENABLE_AAF));

    HAL_Delay(50);

    return HAL_OK;
}
