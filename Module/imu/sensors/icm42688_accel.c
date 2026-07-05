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
    uint8_t _idx = (uint8_t)handle->accel_config.accel_fsr;
    if (_idx > 3U)
        _idx = 0U;

    handle->accel_lsb_per_g_dtsheet = lsb_per_g[_idx];
    handle->accel_g_per_lsb         = 1.0f / handle->accel_lsb_per_g_dtsheet;
}



/*=============================================================================
 *	ACCEL CONFIG / FILTER
 *============================================================================= */
bool
ICM42688_Set_AccelConfig(ICM42688_Handle_t *handle, ICM42688_Accel_Mode_t mode,
                         ICM42688_Accel_ODR_t odr, ICM42688_Accel_FSR_t fsr)
{
    if (!handle)
        return false;

    if ((uint8_t)mode > 3U)
        return false;

    if ((odr == 0x00U) || (uint8_t)odr > (uint8_t)ACCEL_ODR_500Hz)
        return false;

    if ((uint8_t)fsr > (uint8_t)ACCEL_FSR_2g)
        return false;

    // Validate ODR against accel mode; low-power mode supports only the lower ODR range.
    bool _odr_valid = false;

    if (mode == ACCEL_LOW_NOISE) {
        _odr_valid =
            ((odr >= ACCEL_ODR_32KHz) && (odr <= ACCEL_ODR_12Hz5)) || (odr == ACCEL_ODR_500Hz);
    }
    else if (mode == ACCEL_LOW_POWER) {
        _odr_valid = ((odr >= ACCEL_ODR_200Hz) && (odr <= ACCEL_ODR_500Hz));
    }
    else { // Accel is OFF
        _odr_valid = true;
    }

    if (!_odr_valid)
        return false;

    bool _status = true;

    // (1) PWR_MGMT0: set accel mode first because mode transitions have a required settling delay.
    uint8_t               _curr_raw_mode    = (uint8_t)mode;
    bool                  _curr_mode_is_off = (_curr_raw_mode == 0U) || (_curr_raw_mode == 1U);
    ICM42688_Accel_Mode_t _curr_norm_mode =
        _curr_mode_is_off ? ACCEL_OFF : (ICM42688_Accel_Mode_t)_curr_raw_mode;

    ICM42688_Accel_Mode_t _prev_mode        = handle->accel_config.accel_mode;
    bool                  _prev_mode_is_off = (_prev_mode == 0U) || (_prev_mode == 1U);
    ICM42688_Accel_Mode_t _prev_norm_mode   = _prev_mode_is_off ? ACCEL_OFF : _prev_mode;

    bool _need_write_mode = ((!handle->is_initialized) || (_curr_norm_mode != _prev_norm_mode));
    {
        if (_need_write_mode) {
            _status =
                ICM42688_Update_Reg_Bits(handle, ICM42688_UB0_PWR_MGMT0, ICM42688_ACCEL_MODE_Msk,
                                         ICM42688_ACCEL_MODE_Val(mode));
            if (!_status)
                return false;

            handle->accel_config.accel_mode = _curr_norm_mode;

            if (_prev_mode_is_off && !_curr_mode_is_off) {
                HAL_Delay(1); // 1 ms, safely above the 200 us minimum when waking from OFF
            }
        }
    }

    // (2) ACCEL_CONF0: set ODR and FSR together so cached scale factors match the register state.
    bool _need_write_config =
        (!(handle->is_initialized) || (odr != handle->accel_config.accel_odr) ||
         (fsr != handle->accel_config.accel_fsr));
    {
        if (_need_write_config) {
            uint8_t _mask         = ICM42688_ACCEL_ODR_Msk | ICM42688_ACCEL_FS_SEL_Msk;
            uint8_t _value_masked = ICM42688_ACCEL_ODR_Val(odr) | ICM42688_ACCEL_FS_SEL_Val(fsr);

            _status =
                ICM42688_Update_Reg_Bits(handle, ICM42688_UB0_ACCEL_CONF0, _mask, _value_masked);
            if (!_status)
                return false;

            handle->accel_config.accel_odr = odr;
            handle->accel_config.accel_fsr = fsr;

            ICM42688_Update_AccelScaleFactor(handle);
        }
    }
    return true;
}



bool
ICM42688_Get_Accel_Mode(ICM42688_Handle_t *handle, uint8_t *modeInfo)
{
    if (!handle || !modeInfo)
        return false;

    uint8_t           _reg    = 0;
    bool _status = ICM42688_ReadReg(handle, ICM42688_UB0_PWR_MGMT0, &_reg);
    if (!_status)
        return false;

    _reg &= (uint8_t)ICM42688_ACCEL_MODE_Msk;
    uint8_t _raw_mode = (uint8_t)(_reg >> ICM42688_ACCEL_MODE_Pos);

    if ((_raw_mode == 0U) || (_raw_mode == 1U)) {
        *modeInfo                       = 0U;
        handle->accel_config.accel_mode = ACCEL_OFF;
    }
    else if (_raw_mode == 2U) {
        *modeInfo                       = 2U;
        handle->accel_config.accel_mode = ACCEL_LOW_POWER;
    }
    else if (_raw_mode == 3U) {
        *modeInfo                       = 3U;
        handle->accel_config.accel_mode = ACCEL_LOW_NOISE;
    }
    else {
        return false;
    }
    return true;
}



bool
ICM42688_Set_Accel_UIFilt_BW(ICM42688_Handle_t *handle, ICM42688_UIFilt_BW_t uiFiltBandWidth)
{
    if (!handle)
        return false;

    uint8_t _v = (uint8_t)uiFiltBandWidth;

    if (((_v >= 8U) && (_v <= 13U)) || (_v > 0x0F))
        return false;

    uint8_t _reg = 0U;

    bool _status = ICM42688_ReadReg(handle, ICM42688_UB0_GYRO_ACCEL_CONF0, &_reg);
    if (!_status)
        return false;

    _reg &= (uint8_t)~ICM42688_ACCEL_UI_FILT_BW_Msk;

    if (handle->accel_config.accel_mode == ACCEL_LOW_NOISE) {
        if (uiFiltBandWidth == BW_1x_AVG_FILT)
            uiFiltBandWidth = BW_400Hz_ODR_DIV_4;
        else if (uiFiltBandWidth == BW_16x_AVG_FILT)
            uiFiltBandWidth = BW_400Hz_ODR_DIV_20;
        _reg |= ICM42688_ACCEL_UI_FILT_BW_Val(uiFiltBandWidth);
    }

    else if (handle->accel_config.accel_mode == ACCEL_LOW_POWER) {
        if (_v == 1U)
            uiFiltBandWidth = BW_1x_AVG_FILT;
        else if (_v == 6U)
            uiFiltBandWidth = BW_16x_AVG_FILT;
        else
            return false;
        _reg |= ICM42688_ACCEL_UI_FILT_BW_Val((uint8_t)uiFiltBandWidth);
    }

    else
        return false;

    _status = ICM42688_WriteReg(handle, ICM42688_UB0_GYRO_ACCEL_CONF0, _reg);
    if (!_status)
        return false;

    handle->accel_config.accel_uifilt_bw = (ICM42688_UIFilt_BW_t)uiFiltBandWidth;

    return true;
}



bool
ICM42688_Set_Accel_UIFilt_Order(ICM42688_Handle_t            *handle,
                                ICM42688_Accel_UIFilt_Order_t uiFiltOrder)
{
    if (!handle)
        return false;

    if ((uint8_t)uiFiltOrder > (uint8_t)ACCEL_THIRD_ORDER)
        return false;

    uint8_t _reg    = 0U;
    bool    _status = ICM42688_ReadReg(handle, ICM42688_UB0_ACCEL_CONF1, &_reg);
    if (!_status)
        return false;

    _reg &= (uint8_t)~ICM42688_ACCEL_UI_FILT_ORD_Msk;
    _reg |= (uint8_t)ICM42688_ACCEL_UI_FILT_ORD_Val(uiFiltOrder);
    _status = ICM42688_WriteReg(handle, ICM42688_UB0_ACCEL_CONF1, _reg);

    if (!_status)
        return false;

    handle->accel_config.accel_filt_order = uiFiltOrder;

    return true;
}



bool
ICM42688_Set_Accel_Anti_Alias_Filt(ICM42688_Handle_t *handle, ICM42688_AAF_En_t antiAliasState)
{
    if (!handle)
        return false;

    if (((uint8_t)antiAliasState != 0U) && ((uint8_t)antiAliasState != 1U))
        return false;

    uint8_t _reg    = 0U;
    bool    _status = ICM42688_ReadReg(handle, ICM42688_UB2_ACCEL_CONF_STATIC2, &_reg);
    if (!_status)
        return false;

    _reg &= (uint8_t)~ICM42688_ACCEL_AAF_DIS_Msk;
    _reg |= (uint8_t)ICM42688_ACCEL_AAF_DIS_Val(antiAliasState);
    _status = ICM42688_WriteReg(handle, ICM42688_UB2_ACCEL_CONF_STATIC2, _reg);

    if (!_status)
        return false;

    handle->accel_config.accel_aaf_state = antiAliasState;

    return true;
}
