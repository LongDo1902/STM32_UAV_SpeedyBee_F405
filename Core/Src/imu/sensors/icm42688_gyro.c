/*
 * icm42688_gyro.c
 *
 *  Created on: Mar 12, 2026
 *      Author: dobao
 */
#include "imu/sensors/icm42688_gyro.h"
#include "math.h"

static const float lsb_per_dps[] = {16.4f, 32.8f, 65.5f, 131.0f, 262.0f, 524.3f, 1048.6f, 2097.2f};

static inline void
ICM42688_Update_GyroScaleFactor(ICM42688_Handle_t *handle)
{
    uint8_t idx = (uint8_t)handle->gyro_config.gyro_fsr;
    if (idx > 7U)
        idx = 0U;

    handle->gyro_lsb_per_dps_dtsheet = lsb_per_dps[idx];
    handle->gyro_dps_per_lsb         = 1.0f / handle->gyro_lsb_per_dps_dtsheet;
}



/*=============================================================================
 *	GYRO CONFIG / FILTER
 *============================================================================= */
ICM42688_Status_t
ICM42688_Set_GyroConfig(ICM42688_Handle_t *handle, ICM42688_Gyro_Mode_t mode,
                        ICM42688_Gyro_ODR_t odr, ICM42688_Gyro_FSR_t fsr)
{
    if (!handle)
        return ICM42688_ERROR;

    if (((uint8_t)mode > 3U) || ((uint8_t)mode == 2U))
        return ICM42688_ERROR;

    if ((((uint8_t)odr > (uint8_t)GYRO_ODR_500Hz)) || ((uint8_t)odr == 0x00U) ||
        ((uint8_t)odr == 0x0CU) || ((uint8_t)odr == 0x0DU) || ((uint8_t)odr == 0x0EU))
        return ICM42688_ERROR;

    if ((uint8_t)fsr > (uint8_t)GYRO_FSR_15dps625)
        return ICM42688_ERROR;

    HAL_StatusTypeDef status = HAL_OK;

    // (1) PWR_MGMT0: Set Gyro Mode
    ICM42688_Gyro_Mode_t prev_mode       = handle->gyro_config.gyro_mode;
    ICM42688_Gyro_Mode_t curr_mode       = mode;
    bool                 need_write_mode = (!(handle->is_initialized) || (curr_mode != prev_mode));
    {
        // Skip if already cached & initialized
        if (need_write_mode == true) {
            status = ICM42688_Update_Reg_Bits(handle, ICM42688_UB0_PWR_MGMT0,
                                              ICM42688_GYRO_MODE_Msk, ICM42688_GYRO_MODE_Val(mode));
            if (status != HAL_OK)
                return ICM42688_ERROR;

            handle->gyro_config.gyro_mode = mode;

            // Add delay >= 200us according to datasheet
            if ((prev_mode == GYRO_OFF) && (curr_mode != GYRO_OFF)) {
                HAL_Delay(1); // 1000us
            }
        }
    }

    // (2) GYRO_CONF0: Set ODR + FSR together
    bool need_write_conf = (!(handle->is_initialized) || (odr != (handle->gyro_config.gyro_odr)) ||
                            (fsr != (handle->gyro_config.gyro_fsr)));
    {
        if (need_write_conf == true) {
            uint8_t mask         = ICM42688_GYRO_ODR_Msk | ICM42688_GYRO_FS_SEL_Msk;
            uint8_t value_masked = ICM42688_GYRO_ODR_Val(odr) | ICM42688_GYRO_FS_SEL_Val(fsr);

            status = ICM42688_Update_Reg_Bits(handle, ICM42688_UB0_GYRO_CONF0, mask, value_masked);
            if (status != HAL_OK)
                return ICM42688_ERROR;

            // Update cache + scale factor after successful HW write
            handle->gyro_config.gyro_odr = odr;
            handle->gyro_config.gyro_fsr = fsr;

            ICM42688_Update_GyroScaleFactor(handle);
        }
    }
    return ICM42688_OK;
}



ICM42688_Status_t
ICM42688_Get_Gyro_Mode(ICM42688_Handle_t *handle, uint8_t *modeInfo)
{
    if (!handle || !modeInfo)
        return ICM42688_ERROR;

    uint8_t           reg    = 0U;
    HAL_StatusTypeDef status = ICM42688_ReadReg(handle, ICM42688_UB0_PWR_MGMT0, &reg);
    if (status != HAL_OK)
        return ICM42688_ERROR;

    reg &= ICM42688_GYRO_MODE_Msk;
    uint8_t raw_mode = (uint8_t)(reg >> ICM42688_GYRO_MODE_Pos);

    if ((raw_mode == 0U) || (raw_mode == 1U) || (raw_mode == 3U)) {
        *modeInfo                     = raw_mode;
        handle->gyro_config.gyro_mode = (ICM42688_Gyro_Mode_t)raw_mode;
        return ICM42688_OK;
    }
    return ICM42688_ERROR;
}



ICM42688_Status_t
ICM42688_Set_Gyro_UIFilt_BW(ICM42688_Handle_t *handle, ICM42688_UIFilt_BW_t UI_Filt_BandWidth)
{
    if (!handle)
        return ICM42688_ERROR;

    if ((((uint8_t)UI_Filt_BandWidth >= 8U) && ((uint8_t)UI_Filt_BandWidth <= 13U)) ||
        (uint8_t)UI_Filt_BandWidth > 0x0FU)
        return ICM42688_ERROR;

    HAL_StatusTypeDef status = ICM42688_Update_Reg_Bits(
        handle, ICM42688_UB0_GYRO_ACCEL_CONF0, ICM42688_GYRO_UI_FILT_BW_Msk,
        ICM42688_GYRO_UI_FILT_BW_Val((uint8_t)UI_Filt_BandWidth));
    if (status != HAL_OK)
        return ICM42688_ERROR;

    handle->gyro_config.gyro_uifilt_bw = UI_Filt_BandWidth;

    return ICM42688_OK;
}



ICM42688_Status_t
ICM42688_Set_Gyro_UIFilt_Order(ICM42688_Handle_t           *handle,
                               ICM42688_Gyro_UIFilt_Order_t UI_Filt_Order)
{
    if (!handle)
        return ICM42688_ERROR;

    uint8_t           reg    = 0U;
    HAL_StatusTypeDef status = ICM42688_ReadReg(handle, ICM42688_UB0_GYRO_CONF1, &reg);
    if (status != HAL_OK)
        return ICM42688_ERROR;

    reg &= (uint8_t)~ICM42688_GYRO_UI_FILT_ORD_Msk;
    reg |= (uint8_t)ICM42688_GYRO_UI_FILT_ORD_Val(UI_Filt_Order);
    status = ICM42688_WriteReg(handle, ICM42688_UB0_GYRO_CONF1, reg);

    if (status != HAL_OK)
        return ICM42688_ERROR;

    handle->gyro_config.gyro_filt_order = UI_Filt_Order;

    return ICM42688_OK;
}



ICM42688_Status_t
ICM42688_Set_Gyro_Anti_Alias_Filt(ICM42688_Handle_t *handle, ICM42688_AAF_En_t antiAliasState)
{
    if (!handle)
        return ICM42688_ERROR;

    uint8_t           reg    = 0U;
    HAL_StatusTypeDef status = ICM42688_ReadReg(handle, ICM42688_UB1_GYRO_CONF_STATIC2, &reg);
    if (status != HAL_OK)
        return ICM42688_ERROR;

    reg &= (uint8_t)~ICM42688_GYRO_AAF_DIS_Msk;
    reg |= (uint8_t)ICM42688_GYRO_AAF_DIS_Val(antiAliasState);
    status = ICM42688_WriteReg(handle, ICM42688_UB1_GYRO_CONF_STATIC2, reg);

    if (status != HAL_OK)
        return ICM42688_ERROR;

    handle->gyro_config.gyro_aaf_state = antiAliasState;

    return ICM42688_OK;
}



ICM42688_Status_t
ICM42688_Set_Gyro_Notch_Filt(ICM42688_Handle_t *handle, ICM42688_Notch_Filt_En_t notchFiltState)
{
    if (!handle)
        return ICM42688_ERROR;

    uint8_t           reg    = 0U;
    HAL_StatusTypeDef status = ICM42688_ReadReg(handle, ICM42688_UB1_GYRO_CONF_STATIC2, &reg);
    if (status != HAL_OK)
        return ICM42688_ERROR;

    reg &= (uint8_t)~ICM42688_GYRO_NOTCH_FILT_Msk;
    reg |= (uint8_t)ICM42688_GYRO_NOTCH_FILT_Val(notchFiltState);
    status = ICM42688_WriteReg(handle, ICM42688_UB1_GYRO_CONF_STATIC2, reg);

    if (status != HAL_OK)
        return ICM42688_ERROR;

    handle->gyro_config.gyro_notch_filt_state = notchFiltState;

    return ICM42688_OK;
}



static HAL_StatusTypeDef
ICM42688_Compute_NotchFreq(uint16_t desiredNotchFreq_Hz, uint16_t *nf_coswz, uint8_t *nf_coswz_sel)
{
    if (!nf_coswz || !nf_coswz_sel)
        return HAL_ERROR;

    if ((desiredNotchFreq_Hz < 1000U) || (desiredNotchFreq_Hz > 3000U))
        return HAL_ERROR;

    float desiredNotchFreq_kHz = (float)(desiredNotchFreq_Hz / 1000.0f);
    float coswz                = cosf(2.0f * M_PI * desiredNotchFreq_kHz / 32.0f);

    if (fabsf(coswz) <= 0.875f) {
        *nf_coswz     = (uint16_t)lroundf(coswz * 256.0f);
        *nf_coswz_sel = 0U;
        return HAL_OK;
    }
    else {
        *nf_coswz_sel = 1U;
        if (coswz > 0.875f) {
            *nf_coswz = (uint16_t)lroundf(8.0f * (1.0f - coswz) * 256.0f);
            return HAL_OK;
        }
        else if (coswz < -0.875f) {
            *nf_coswz = (uint16_t)lroundf(-8.0f * (1.0f + coswz) * 256.0f);
            return HAL_OK;
        }
    }
    return HAL_ERROR;
}



static HAL_StatusTypeDef
ICM42688_Set_NotchFreq_X(ICM42688_Handle_t *handle, uint16_t desiredNotchFreq_Hz)
{
    if (!handle)
        return HAL_ERROR;

    // Compute desired notch frequency and convert it to integer
    uint16_t          nf_coswz_x     = 0U;
    uint8_t           nf_coswz_x_sel = 0U;
    HAL_StatusTypeDef status =
        ICM42688_Compute_NotchFreq(desiredNotchFreq_Hz, &nf_coswz_x, &nf_coswz_x_sel);
    if (status != HAL_OK)
        return status;

    // Write nf_coswz_x[7:0] to GYRO_CONF_STATIC6 register
    uint8_t regLow  = 0U;
    uint8_t regHigh = 0U;

    regLow = (uint8_t)ICM42688_GYRO_X_NF_COSWZ_LOW_Val(nf_coswz_x);
    status = ICM42688_WriteReg(handle, ICM42688_UB1_GYRO_CONF_STATIC6, regLow);
    if (status != HAL_OK)
        return status;

    // Write nf_coswz_x (bit 8) and and nf_coswz_x_sel to the GYRO_CONF_STATIC9 register
    status = ICM42688_ReadReg(handle, ICM42688_UB1_GYRO_CONF_STATIC9, &regHigh);
    if (status != HAL_OK)
        return status;

    regHigh &= (uint8_t)~(ICM42688_GYRO_X_NF_COSWZ_HIGH_Msk | ICM42688_GYRO_X_NF_COSWZ_SEL_Msk);
    regHigh |= (uint8_t)(ICM42688_GYRO_X_NF_COSWZ_HIGH_Val((nf_coswz_x >> 8U) & 0x01U) |
                         ICM42688_GYRO_X_NF_COSWZ_SEL_Val(nf_coswz_x_sel));
    status = ICM42688_WriteReg(handle, ICM42688_UB1_GYRO_CONF_STATIC9, regHigh);
    if (status != HAL_OK)
        return status;

    return HAL_OK;
}



static HAL_StatusTypeDef
ICM42688_Set_NotchFreq_Y(ICM42688_Handle_t *handle, uint16_t desiredNotchFreq_Hz)
{
    if (!handle)
        return HAL_ERROR;

    // Compute desired notch frequency and convert it to integer
    uint16_t          nf_coswz_y     = 0U;
    uint8_t           nf_coswz_y_sel = 0U;
    HAL_StatusTypeDef status =
        ICM42688_Compute_NotchFreq(desiredNotchFreq_Hz, &nf_coswz_y, &nf_coswz_y_sel);
    if (status != HAL_OK)
        return status;

    // Write nf_coswz_y[7:0] to GYRO_CONF_STATIC7 register
    uint8_t regLow  = 0U;
    uint8_t regHigh = 0U;

    regLow = (uint8_t)ICM42688_GYRO_Y_NF_COSWZ_LOW_Val(nf_coswz_y);
    status = ICM42688_WriteReg(handle, ICM42688_UB1_GYRO_CONF_STATIC7, regLow);
    if (status != HAL_OK)
        return status;

    // Write nf_coswz_y (bit 8) and and nf_coswz_y_sel to the GYRO_CONF_STATIC9 register
    status = ICM42688_ReadReg(handle, ICM42688_UB1_GYRO_CONF_STATIC9, &regHigh);
    if (status != HAL_OK)
        return status;

    regHigh &= (uint8_t)~(ICM42688_GYRO_Y_NF_COSWZ_HIGH_Msk | ICM42688_GYRO_Y_NF_COSWZ_SEL_Msk);
    regHigh |= (uint8_t)(ICM42688_GYRO_Y_NF_COSWZ_HIGH_Val((nf_coswz_y >> 8U) & 0x01U) |
                         ICM42688_GYRO_Y_NF_COSWZ_SEL_Val(nf_coswz_y_sel));
    status = ICM42688_WriteReg(handle, ICM42688_UB1_GYRO_CONF_STATIC9, regHigh);
    if (status != HAL_OK)
        return status;

    return HAL_OK;
}



static HAL_StatusTypeDef
ICM42688_Set_NotchFreq_Z(ICM42688_Handle_t *handle, uint16_t desiredNotchFreq_Hz)
{
    if (!handle)
        return HAL_ERROR;

    // Compute desired notch frequency and convert it to integer
    uint16_t          nf_coswz_z     = 0U;
    uint8_t           nf_coswz_z_sel = 0U;
    HAL_StatusTypeDef status =
        ICM42688_Compute_NotchFreq(desiredNotchFreq_Hz, &nf_coswz_z, &nf_coswz_z_sel);
    if (status != HAL_OK)
        return status;

    // Write nf_coswz_z[7:0] to GYRO_CONF_STATIC8 register
    uint8_t regLow  = 0U;
    uint8_t regHigh = 0U;

    regLow = (uint8_t)ICM42688_GYRO_Z_NF_COSWZ_LOW_Val(nf_coswz_z);
    status = ICM42688_WriteReg(handle, ICM42688_UB1_GYRO_CONF_STATIC8, regLow);
    if (status != HAL_OK)
        return status;

    // Write nf_coswz_z (bit 8) and and nf_coswz_z_sel to the GYRO_CONF_STATIC9 register
    status = ICM42688_ReadReg(handle, ICM42688_UB1_GYRO_CONF_STATIC9, &regHigh);
    if (status != HAL_OK)
        return status;

    regHigh &= (uint8_t)~(ICM42688_GYRO_Z_NF_COSWZ_HIGH_Msk | ICM42688_GYRO_Z_NF_COSWZ_SEL_Msk);
    regHigh |= (uint8_t)(ICM42688_GYRO_Z_NF_COSWZ_HIGH_Val((nf_coswz_z >> 8U) & 0x01U) |
                         ICM42688_GYRO_Z_NF_COSWZ_SEL_Val(nf_coswz_z_sel));
    status = ICM42688_WriteReg(handle, ICM42688_UB1_GYRO_CONF_STATIC9, regHigh);
    if (status != HAL_OK)
        return status;

    return HAL_OK;
}


HAL_StatusTypeDef
ICM42688_Set_NotchFreq_XYZ(ICM42688_Handle_t *handle, uint16_t desired_X_NotchFreq_Hz,
                           uint16_t desired_Y_NotchFreq_Hz, uint16_t desired_Z_NotchFreq_Hz)
{
    if (!handle)
        return HAL_ERROR;

    HAL_StatusTypeDef status = ICM42688_Set_NotchFreq_X(handle, desired_X_NotchFreq_Hz);
    if (status != HAL_OK)
        return status;

    status = ICM42688_Set_NotchFreq_Y(handle, desired_Y_NotchFreq_Hz);
    if (status != HAL_OK)
        return status;

    status = ICM42688_Set_NotchFreq_Z(handle, desired_Z_NotchFreq_Hz);
    if (status != HAL_OK)
        return status;

    return HAL_OK;
}