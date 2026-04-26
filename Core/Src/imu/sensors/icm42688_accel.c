/*
 * icm42688_accel.c
 *
 *  Created on: Mar 12, 2026
 *      Author: dobao
 */

#include "imu/sensors/icm42688_accel.h"

static const float lsb_per_g[] = {2048.0f, 4096.0f, 8192.0f, 16384.0f};

static inline void
ICM42688_Update_AccelScaleFactor(ICM42688_Handle_t *handle)
{
    uint8_t idx = (uint8_t)handle->accel_config.accel_fsr;
    if (idx > 3U)
        idx = 0U;

    handle->accel_lsb_per_g_dtsheet = lsb_per_g[idx];
    handle->accel_g_per_lsb         = 1.0f / handle->accel_lsb_per_g_dtsheet;
}



/*=============================================================================
 *	ACCEL CONFIG / FILTER
 *============================================================================= */
ICM42688_Status_t
ICM42688_Set_AccelConfig(ICM42688_Handle_t *handle, ICM42688_Accel_Mode_t mode,
                         ICM42688_Accel_ODR_t odr, ICM42688_Accel_FSR_t fsr)
{
    if (!handle)
        return ICM42688_ERROR;

    if ((uint8_t)mode > 3U)
        return ICM42688_ERROR;

    if ((odr == 0x00U) || (uint8_t)odr > (uint8_t)ACCEL_ODR_500Hz)
        return ICM42688_ERROR;

    if ((uint8_t)fsr > (uint8_t)ACCEL_FSR_2g)
        return ICM42688_ERROR;

    // Validate ODR against accel mode
    bool odr_valid = false;

    if (mode == ACCEL_LOW_NOISE) {
        odr_valid =
            ((odr >= ACCEL_ODR_32KHz) && (odr <= ACCEL_ODR_12Hz5)) || (odr == ACCEL_ODR_500Hz);
    }
    else if (mode == ACCEL_LOW_POWER) {
        odr_valid = ((odr >= ACCEL_ODR_200Hz) && (odr <= ACCEL_ODR_500Hz));
    }
    else { // Accel is OFF
        odr_valid = true;
    }

    if (!odr_valid)
        return ICM42688_ERROR;

    HAL_StatusTypeDef status = HAL_OK;

    // (1) PWR_MGMT0: set Accel Mode (RMW)
    uint8_t               curr_raw_mode    = (uint8_t)mode;
    bool                  curr_mode_is_off = (curr_raw_mode == 0U) || (curr_raw_mode == 1U);
    ICM42688_Accel_Mode_t curr_norm_mode =
        curr_mode_is_off ? ACCEL_OFF : (ICM42688_Accel_Mode_t)curr_raw_mode;

    ICM42688_Accel_Mode_t prev_mode        = handle->accel_config.accel_mode;
    bool                  prev_mode_is_off = (prev_mode == 0U) || (prev_mode == 1U);
    ICM42688_Accel_Mode_t prev_norm_mode   = prev_mode_is_off ? ACCEL_OFF : prev_mode;

    bool need_write_mode = ((!handle->is_initialized) || (curr_norm_mode != prev_norm_mode));
    {
        if (need_write_mode) {
            status =
                ICM42688_Update_Reg_Bits(handle, ICM42688_UB0_PWR_MGMT0, ICM42688_ACCEL_MODE_Msk,
                                         ICM42688_ACCEL_MODE_Val(mode));
            if (status != HAL_OK)
                return ICM42688_ERROR;

            handle->accel_config.accel_mode = curr_norm_mode;

            if (prev_mode_is_off && !curr_mode_is_off) {
                HAL_Delay(1); // Wait at least 200us if mode changes from OFF to any other mode
            }
        }
    }

    // (2) ACCEL_CONF0: set ODR and FSR together
    bool need_write_config =
        (!(handle->is_initialized) || (odr != handle->accel_config.accel_odr) ||
         (fsr != handle->accel_config.accel_fsr));
    {
        if (need_write_config) {
            uint8_t mask         = ICM42688_ACCEL_ODR_Msk | ICM42688_ACCEL_FS_SEL_Msk;
            uint8_t value_masked = ICM42688_ACCEL_ODR_Val(odr) | ICM42688_ACCEL_FS_SEL_Val(fsr);

            status = ICM42688_Update_Reg_Bits(handle, ICM42688_UB0_ACCEL_CONF0, mask, value_masked);
            if (status != HAL_OK)
                return ICM42688_ERROR;

            handle->accel_config.accel_odr = odr;
            handle->accel_config.accel_fsr = fsr;

            ICM42688_Update_AccelScaleFactor(handle);
        }
    }
    return ICM42688_OK;
}



ICM42688_Status_t
ICM42688_Get_Accel_Mode(ICM42688_Handle_t *handle, uint8_t *modeInfo)
{
    if (!handle || !modeInfo)
        return ICM42688_ERROR;

    uint8_t           reg    = 0;
    HAL_StatusTypeDef status = ICM42688_ReadReg(handle, ICM42688_UB0_PWR_MGMT0, &reg);
    if (status != HAL_OK)
        return ICM42688_ERROR;

    reg &= (uint8_t)ICM42688_ACCEL_MODE_Msk;
    uint8_t raw_mode = (uint8_t)(reg >> ICM42688_ACCEL_MODE_Pos);

    if ((raw_mode == 0U) || (raw_mode == 1U)) {
        *modeInfo                       = 0U;
        handle->accel_config.accel_mode = ACCEL_OFF;
    }
    else if (raw_mode == 2U) {
        *modeInfo                       = 2U;
        handle->accel_config.accel_mode = ACCEL_LOW_POWER;
    }
    else if (raw_mode == 3U) {
        *modeInfo                       = 3U;
        handle->accel_config.accel_mode = ACCEL_LOW_NOISE;
    }
    else {
        return ICM42688_ERROR;
    }
    return ICM42688_OK;
}



ICM42688_Status_t
ICM42688_Set_Accel_UIFilt_BW(ICM42688_Handle_t *handle, ICM42688_UIFilt_BW_t UI_Filt_BandWidth)
{
    if (!handle)
        return ICM42688_ERROR;

    uint8_t v = (uint8_t)UI_Filt_BandWidth;

    if (((v >= 8U) && (v <= 13U)) || (v > 0x0F))
        return ICM42688_ERROR;

    uint8_t reg = 0U;

    HAL_StatusTypeDef status = ICM42688_ReadReg(handle, ICM42688_UB0_GYRO_ACCEL_CONF0, &reg);
    if (status != HAL_OK)
        return ICM42688_ERROR;

    reg &= (uint8_t)~ICM42688_ACCEL_UI_FILT_BW_Msk;

    if (handle->accel_config.accel_mode == ACCEL_LOW_NOISE) {
        if (UI_Filt_BandWidth == BW_1x_AVG_FILT)
            UI_Filt_BandWidth = BW_400Hz_ODR_DIV_4;
        else if (UI_Filt_BandWidth == BW_16x_AVG_FILT)
            UI_Filt_BandWidth = BW_400Hz_ODR_DIV_20;
        reg |= ICM42688_ACCEL_UI_FILT_BW_Val(UI_Filt_BandWidth);
    }

    else if (handle->accel_config.accel_mode == ACCEL_LOW_POWER) {
        if (v == 1U)
            UI_Filt_BandWidth = BW_1x_AVG_FILT;
        else if (v == 6U)
            UI_Filt_BandWidth = BW_16x_AVG_FILT;
        else
            return ICM42688_ERROR;
        reg |= ICM42688_ACCEL_UI_FILT_BW_Val((uint8_t)UI_Filt_BandWidth);
    }

    else
        return ICM42688_ERROR;

    status = ICM42688_WriteReg(handle, ICM42688_UB0_GYRO_ACCEL_CONF0, reg);
    if (status != HAL_OK)
        return ICM42688_ERROR;

    handle->accel_config.accel_uifilt_bw = (ICM42688_UIFilt_BW_t)UI_Filt_BandWidth;

    return ICM42688_OK;
}



ICM42688_Status_t
ICM42688_Set_Accel_UIFilt_Order(ICM42688_Handle_t            *handle,
                                ICM42688_Accel_UIFilt_Order_t UI_Filt_Order)
{
    if (!handle)
        return ICM42688_ERROR;

    uint8_t           reg    = 0U;
    HAL_StatusTypeDef status = ICM42688_ReadReg(handle, ICM42688_UB0_ACCEL_CONF1, &reg);
    if (status != HAL_OK)
        return ICM42688_ERROR;

    reg &= (uint8_t)~ICM42688_ACCEL_UI_FILT_ORD_Msk;
    reg |= (uint8_t)ICM42688_ACCEL_UI_FILT_ORD_Val(UI_Filt_Order);
    status = ICM42688_WriteReg(handle, ICM42688_UB0_ACCEL_CONF1, reg);

    if (status != HAL_OK)
        return ICM42688_ERROR;

    handle->accel_config.accel_filt_order = UI_Filt_Order;

    return ICM42688_OK;
}



ICM42688_Status_t
ICM42688_Set_Accel_Anti_Alias_Filt(ICM42688_Handle_t *handle, ICM42688_AAF_En_t antiAliasState)
{
    if (!handle)
        return ICM42688_ERROR;

    uint8_t           reg    = 0U;
    HAL_StatusTypeDef status = ICM42688_ReadReg(handle, ICM42688_UB2_ACCEL_CONF_STATIC2, &reg);
    if (status != HAL_OK)
        return ICM42688_ERROR;

    reg &= (uint8_t)~ICM42688_ACCEL_AAF_DIS_Msk;
    reg |= (uint8_t)ICM42688_ACCEL_AAF_DIS_Val(antiAliasState);
    status = ICM42688_WriteReg(handle, ICM42688_UB2_ACCEL_CONF_STATIC2, reg);

    if (status != HAL_OK)
        return ICM42688_ERROR;

    handle->accel_config.accel_aaf_state = antiAliasState;

    return ICM42688_OK;
}
