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
HAL_StatusTypeDef
ICM42688_Set_AccelConfig(ICM42688_Handle_t *handle, ICM42688_AccelMode_t mode,
                         ICM42688_AccelODR_t odr, ICM42688_AccelFSR_t fsr)
{
    if (!handle)
        return HAL_ERROR;
    if ((uint8_t)mode > 3U)
        return HAL_ERROR;
    if ((odr == 0x00U) || (uint8_t)odr > (uint8_t)ACCEL_ODR_500Hz)
        return HAL_ERROR;
    if ((uint8_t)fsr > (uint8_t)ACCEL_FSR_2g)
        return HAL_ERROR;

    // Validate ODR against accel mode
    bool odrValid = false;
    if (mode == ACCEL_LOW_NOISE) {
        odrValid =
            ((odr >= ACCEL_ODR_32KHz) && (odr <= ACCEL_ODR_12Hz5)) || (odr == ACCEL_ODR_500Hz);
    }
    else if (mode == ACCEL_LOW_POWER) {
        odrValid = ((odr >= ACCEL_ODR_200Hz) && (odr <= ACCEL_ODR_500Hz));
    }
    else { // Accel is OFF
        odrValid = true;
    }

    if (!odrValid)
        return HAL_ERROR;

    HAL_StatusTypeDef status = HAL_OK;

    // (1) PWR_MGMT0: set Accel Mode (RMW)
    uint8_t              currRawMode   = (uint8_t)mode;
    bool                 currModeIsOff = (currRawMode == 0U) || (currRawMode == 1U);
    ICM42688_AccelMode_t currNormMode =
        currModeIsOff ? ACCEL_OFF : (ICM42688_AccelMode_t)currRawMode;

    ICM42688_AccelMode_t prevMode      = handle->accel_config.accel_mode;
    bool                 prevModeIsOff = (prevMode == 0U) || (prevMode == 1U);
    ICM42688_AccelMode_t prevNormMode  = prevModeIsOff ? ACCEL_OFF : prevMode;

    bool needWriteMode = ((!handle->is_initialized) || (currNormMode != prevNormMode));
    {
        if (needWriteMode) {
            status =
                ICM42688_Update_Reg_Bits(handle, ICM42688_UB0_PWR_MGMT0, ICM42688_ACCEL_MODE_Msk,
                                         ICM42688_ACCEL_MODE_Val(mode));
            if (status != HAL_OK)
                return status;
            handle->accel_config.accel_mode = currNormMode;

            if (prevModeIsOff && !currModeIsOff) {
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
            uint8_t mask        = ICM42688_ACCEL_ODR_Msk | ICM42688_ACCEL_FS_SEL_Msk;
            uint8_t valueMasked = ICM42688_ACCEL_ODR_Val(odr) | ICM42688_ACCEL_FS_SEL_Val(fsr);

            status = ICM42688_Update_Reg_Bits(handle, ICM42688_UB0_ACCEL_CONF0, mask, valueMasked);
            if (status != HAL_OK)
                return status;

            handle->accel_config.accel_odr = odr;
            handle->accel_config.accel_fsr = fsr;
            ICM42688_Update_AccelScaleFactor(handle);
        }
    }
    return HAL_OK;
}

HAL_StatusTypeDef
ICM42688_Get_Accel_Mode(ICM42688_Handle_t *handle, uint8_t *modeInfo)
{
    if (!handle || !modeInfo)
        return HAL_ERROR;

    uint8_t           reg    = 0;
    HAL_StatusTypeDef status = ICM42688_ReadReg(handle, ICM42688_UB0_PWR_MGMT0, &reg);
    if (status != HAL_OK)
        return status;

    reg &= (uint8_t)ICM42688_ACCEL_MODE_Msk;
    uint8_t rawMode = (uint8_t)(reg >> ICM42688_ACCEL_MODE_Pos);

    if ((rawMode == 0U) || (rawMode == 1U)) {
        *modeInfo                       = 0U;
        handle->accel_config.accel_mode = ACCEL_OFF;
    }
    else if (rawMode == 2U) {
        *modeInfo                       = 2U;
        handle->accel_config.accel_mode = ACCEL_LOW_POWER;
    }
    else if (rawMode == 3U) {
        *modeInfo                       = 3U;
        handle->accel_config.accel_mode = ACCEL_LOW_NOISE;
    }
    else {
        return HAL_ERROR;
    }
    return HAL_OK;
}

HAL_StatusTypeDef
ICM42688_Set_Accel_UIFilt_BW(ICM42688_Handle_t *handle, ICM42688_UIFilt_BW_t bw)
{
    if (!handle)
        return HAL_ERROR;
    uint8_t v = (uint8_t)bw;
    if (((v >= 8U) && (v <= 13U)) || (v > 0x0F))
        return HAL_ERROR;

    uint8_t           reg    = 0U;
    HAL_StatusTypeDef status = ICM42688_ReadReg(handle, ICM42688_UB0_GYRO_ACCEL_CONF0, &reg);
    if (status != HAL_OK)
        return status;
    reg &= (uint8_t)~ICM42688_ACCEL_UI_FILT_BW_Msk;

    if (handle->accel_config.accel_mode == ACCEL_LOW_NOISE) {
        if (bw == BW_1x_AVG_FILT)
            bw = BW_400Hz_ODR_DIV_4;
        else if (bw == BW_16x_AVG_FILT)
            bw = BW_400Hz_ODR_DIV_20;
        reg |= ICM42688_ACCEL_UI_FILT_BW_Val((uint8_t)bw);
    }

    else if (handle->accel_config.accel_mode == ACCEL_LOW_POWER) {
        if (v == 1U)
            bw = BW_1x_AVG_FILT;
        else if (v == 6U)
            bw = BW_16x_AVG_FILT;
        else
            return HAL_ERROR;
        reg |= ICM42688_ACCEL_UI_FILT_BW_Val((uint8_t)bw);
    }

    else
        return HAL_ERROR;

    status = ICM42688_WriteReg(handle, ICM42688_UB0_GYRO_ACCEL_CONF0, reg);
    if (status != HAL_OK)
        return status;

    handle->accel_config.accel_uifilt_bw = (ICM42688_UIFilt_BW_t)bw;
    return HAL_OK;
}

HAL_StatusTypeDef
ICM42688_Set_Accel_UIFilt_Order(ICM42688_Handle_t *handle, ICM42688_AccelUIFiltOrder_t filterOrder)
{
    if (!handle)
        return HAL_ERROR;

    uint8_t           reg    = 0U;
    HAL_StatusTypeDef status = ICM42688_ReadReg(handle, ICM42688_UB0_ACCEL_CONF1, &reg);
    if (status != HAL_OK)
        return status;

    reg &= (uint8_t)~ICM42688_ACCEL_UI_FILT_ORD_Msk;
    reg |= (uint8_t)ICM42688_ACCEL_UI_FILT_ORD_Val(filterOrder);
    status = ICM42688_WriteReg(handle, ICM42688_UB0_ACCEL_CONF1, reg);

    if (status != HAL_OK)
        return status;

    handle->accel_config.accel_filt_order = filterOrder;

    return HAL_OK;
}

HAL_StatusTypeDef
ICM42688_Set_Accel_Anti_Alias_Filt(ICM42688_Handle_t *handle, ICM42688_AAF_En_t antiAliasState)
{
    if (!handle)
        return HAL_ERROR;

    uint8_t           reg    = 0U;
    HAL_StatusTypeDef status = ICM42688_ReadReg(handle, ICM42688_UB2_ACCEL_CONF_STATIC2, &reg);
    if (status != HAL_OK)
        return status;

    reg &= (uint8_t)~ICM42688_ACCEL_AAF_DIS_Msk;
    reg |= (uint8_t)ICM42688_ACCEL_AAF_DIS_Val(antiAliasState);
    status = ICM42688_WriteReg(handle, ICM42688_UB2_ACCEL_CONF_STATIC2, reg);

    if (status != HAL_OK)
        return status;

    handle->accel_config.accel_aaf_state = antiAliasState;

    return HAL_OK;
}
