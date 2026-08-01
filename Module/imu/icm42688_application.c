/*
 * icm42688_application.c
 *
 *  Created on: Mar 12, 2026
 *      Author: dobaolong
 */

#include "imu/icm42688_application.h"

/*=============================================================================
 *	IDENTITY / RESET
 *============================================================================ */
static inline bool
ICM42688_Get_WhoAmI(ICM42688_Handle_t *pHandle, uint8_t *pWhoVal)
{
    if (!pHandle || !pWhoVal)
        return false;
    return ICM42688_ReadReg(pHandle, ICM42688_UB0_WHO_AM_I, pWhoVal);
}



bool
ICM42688_IsAlive(ICM42688_Handle_t *pHandle)
{
    if (!pHandle)
        return false;

    uint8_t _who    = 0U;
    bool    _status = ICM42688_Get_WhoAmI(pHandle, &_who);

    if (!_status) {
        pHandle->is_icm42688_alive = false;
        return false;
    }

    if (_who == ICM42688_WHO_AM_I_DEFAULT) {
        pHandle->is_icm42688_alive = true;
        return true;
    }

    pHandle->is_icm42688_alive = false;
    return false;
}



bool
ICM42688_SoftReset(ICM42688_Handle_t *pHandle)
{
    if (!pHandle)
        return false;

    bool _status =
        ICM42688_Update_Reg_Bits(pHandle, ICM42688_UB0_DEVICE_CONF, ICM42688_DEVICE_CONFIG_SOFT_RESET_Msk,
                                 ICM42688_DEVICE_CONFIG_SOFT_RESET_Msk);
    if (!_status)
        return false;

    HAL_Delay(5);

    // Mirror the post-reset software state so the next init writes every required register.
    pHandle->is_reset          = true;
    pHandle->is_initialized    = false;
    pHandle->is_icm42688_alive = false;

    pHandle->gyro_dps_per_lsb         = 0.0f;
    pHandle->gyro_lsb_per_dps_dtsheet = 0.0f;

    pHandle->accel_g_per_lsb         = 0.0f;
    pHandle->accel_lsb_per_g_dtsheet = 0.0f;

    pHandle->temp_config.temp_state = TEMP_ENABLE;

    pHandle->gyro_config.gyro_odr        = GYRO_ODR_1KHz;
    pHandle->gyro_config.gyro_fsr        = GYRO_FSR_2000dps;
    pHandle->gyro_config.gyro_notch      = GYRO_NOTCHBW_680Hz;
    pHandle->gyro_config.gyro_filt_order = GYRO_SECOND_ORDER;
    pHandle->gyro_config.gyro_mode       = GYRO_OFF;
    pHandle->gyro_config.gyro_uifilt_bw  = BW_400Hz_ODR_DIV_4;

    pHandle->accel_config.accel_odr        = ACCEL_ODR_1KHz;
    pHandle->accel_config.accel_fsr        = ACCEL_FSR_16g;
    pHandle->accel_config.accel_filt_order = ACCEL_SECOND_ORDER;
    pHandle->accel_config.accel_mode       = ACCEL_OFF;
    pHandle->accel_config.accel_uifilt_bw  = BW_400Hz_ODR_DIV_4;

    pHandle->int1_config.int1_polarity = INT_ACTIVE_LOW;
    pHandle->int1_config.int1_drive    = INT_OPEN_DRAIN;
    pHandle->int1_config.int1_mode     = INT_PULSED;

    pHandle->int2_config.int2_polarity = INT_ACTIVE_LOW;
    pHandle->int2_config.int2_drive    = INT_OPEN_DRAIN;
    pHandle->int2_config.int2_mode     = INT_PULSED;

    pHandle->intf_config.ui_sifs_config     = UI_SIFS_RESERVED;
    pHandle->intf_config.sensor_data_endian = SENSOR_DATA_BIG_ENDIAN;

    pHandle->fifo_config.fifo_mode         = BYPASS;
    pHandle->fifo_config.fifo_count_endian = FIFO_COUNT_BIG_ENDIAN;
    pHandle->fifo_config.fifo_count_rec    = FIFO_COUNT_IN_BYTE;

    pHandle->fifo_config.fifo_gyro_state  = FIFO_GAT_ENABLE;
    pHandle->fifo_config.fifo_accel_state = FIFO_GAT_ENABLE;
    pHandle->fifo_config.fifo_temp_state  = FIFO_GAT_ENABLE;

    pHandle->fifo_config.fifo_wm_mode            = FIFO_WM_GREATER_THS_ONESHOT;
    pHandle->fifo_config.fifo_hires_state        = FIFO_HIRES_DISABLE;
    pHandle->fifo_config.fifo_partial_read_state = FIFO_PARTIAL_READ_DISABLE;

    return _status;
}



static bool
ICM42688_Interface_Config(ICM42688_Handle_t *pHandle)
{
    bool _status = false;

    CHECK_FOR(ICM42688_Set_SPI_Mode(pHandle, SPI_MODE_0_3));
    CHECK_FOR(ICM42688_Set_SPI_SlewRate(pHandle, SPI_SR_2NS));
    CHECK_FOR(ICM42688_Set_UI_SIFS_Conf(pHandle, UI_SIFS_DISABLE_I2C));
    CHECK_FOR(ICM42688_Set_Sensor_Data_Endian(pHandle, SENSOR_DATA_BIG_ENDIAN));

    return true;
}



static bool
ICM42688_Accel_Config(ICM42688_Handle_t *pHandle)
{
    bool _status = false;

    CHECK_FOR(ICM42688_Set_AccelConfig(pHandle, ACCEL_LOW_NOISE, ACCEL_ODR_8KHz, ACCEL_FSR_4g));
    CHECK_FOR(ICM42688_Set_Accel_UIFilt_BW(pHandle, BW_ODR_DIV_2));
    CHECK_FOR(ICM42688_Set_Accel_UIFilt_Order(pHandle, ACCEL_FIRST_ORDER));
    CHECK_FOR(ICM42688_Set_Accel_Anti_Alias_Filt(pHandle, ENABLE_AAF));

    return true;
}



static bool
ICM42688_Gyro_Config(ICM42688_Handle_t *pHandle)
{
    bool _status = false;

    CHECK_FOR(ICM42688_Set_GyroConfig(pHandle, GYRO_LOW_NOISE, GYRO_ODR_8KHz, GYRO_FSR_1000dps));
    CHECK_FOR(ICM42688_Set_Gyro_UIFilt_BW(pHandle, BW_ODR_DIV_2));
    CHECK_FOR(ICM42688_Set_Gyro_UIFilt_Order(pHandle, GYRO_FIRST_ORDER));
    CHECK_FOR(ICM42688_Set_Gyro_Anti_Alias_Filt(pHandle, ENABLE_AAF));

    return true;
}



static bool
ICM42688_Temperature_Config(ICM42688_Handle_t *pHandle)
{
    bool _status = false;
    CHECK_FOR(ICM42688_Set_Temperature_Enable(pHandle, TEMP_ENABLE));
    return true;
}



static bool
ICM42688_FIFO_Config(ICM42688_Handle_t *pHandle)
{
    bool _status = false;
    /**
     * @note FIFO watermark is intentionally not configured here; choose it near the consumer
     *       because it depends on the selected read mode, scheduler period, and buffer size.
     */
    CHECK_FOR(ICM42688_Set_FIFO_Count_Endian(pHandle, FIFO_COUNT_BIG_ENDIAN));
    CHECK_FOR(ICM42688_Set_FIFO_Count_Rec(pHandle, FIFO_COUNT_IN_BYTE));
    CHECK_FOR(ICM42688_Set_FIFO_Mode(pHandle, STREAM_TO_FIFO));
    CHECK_FOR(ICM42688_Set_FIFO_Gyro_Enable(pHandle, FIFO_GAT_ENABLE));
    CHECK_FOR(ICM42688_Set_FIFO_Accel_Enable(pHandle, FIFO_GAT_ENABLE));

    CHECK_FOR(ICM42688_Set_FIFO_Temp_Enable(pHandle, FIFO_GAT_ENABLE));
    CHECK_FOR(ICM42688_Set_FIFO_HIRES_Enable(pHandle, FIFO_HIRES_DISABLE));
    CHECK_FOR(ICM42688_Set_FIFO_Resume_Partial_Read(pHandle, FIFO_PARTIAL_READ_DISABLE));

    return true;
}



bool
ICM42688_Init(ICM42688_Handle_t *pHandle)
{
    if (!pHandle)
        return false;

    bool _status = false;

    // Verify SPI communication before changing device configuration.
    CHECK_FOR(ICM42688_IsAlive(pHandle));

    // Request soft reset and poll INT_RESET_DONE for up to 100 ms.
    {
        CHECK_FOR(ICM42688_Set_Int1_ResetDone_Enable(pHandle, true));
        CHECK_FOR(ICM42688_SoftReset(pHandle));

        uint8_t  _int_status            = 0U;
        uint32_t _reset_done_timeout_ms = 100U;

        // Wait until the device reports reset completion.
        do {
            CHECK_FOR(ICM42688_Get_Int_Status(pHandle, &_int_status));
            if (ICM42688_Int_Status_Has(_int_status, INT_RESET_DONE)) {
                break; // Reset is done.
            }
            HAL_Delay(1U);
        } while (--_reset_done_timeout_ms > 0U);

        // If INT_RESET_DONE was not observed within 100 ms, treat reset as failed.
        if (_reset_done_timeout_ms == 0U) {
            return false;
        }
    }

    // Verify the device is reachable again after reset.
    CHECK_FOR(ICM42688_IsAlive(pHandle));

    // Interface configuration.
    CHECK_FOR(ICM42688_Interface_Config(pHandle));

    // Accel configuration.
    CHECK_FOR(ICM42688_Accel_Config(pHandle));

    // Gyro configuration.
    CHECK_FOR(ICM42688_Gyro_Config(pHandle));

    // Temperature configuration.
    CHECK_FOR(ICM42688_Temperature_Config(pHandle));

    // FIFO configuration.
    CHECK_FOR(ICM42688_FIFO_Config(pHandle));

    HAL_Delay(50);

    pHandle->is_initialized    = true;
    pHandle->is_icm42688_alive = true;

    return true;
}
