/*
 * icm42688_gyro.c
 *
 *  Created on: Mar 12, 2026
 *      Author: dobaolong
 */
#include "imu/sensors/icm42688_gyro.h"
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static const float _lsb_per_dps[] = {16.4f, 32.8f, 65.5f, 131.0f, 262.0f, 524.3f, 1048.6f, 2097.2f};

static inline void
ICM42688_Update_GyroScaleFactor(ICM42688_Handle_t *pHandle)
{
    uint8_t _idx = (uint8_t)pHandle->gyro_config.gyro_fsr;
    if (_idx > 7U)
        _idx = 0U;

    pHandle->gyro_lsb_per_dps_dtsheet = _lsb_per_dps[_idx];
    pHandle->gyro_dps_per_lsb         = 1.0f / pHandle->gyro_lsb_per_dps_dtsheet;
}



/*=============================================================================
 *	GYRO CONFIG / FILTER
 *============================================================================= */
bool
ICM42688_Set_GyroConfig(ICM42688_Handle_t *pHandle, ICM42688_Gyro_Mode_t mode, ICM42688_Gyro_ODR_t odr,
                        ICM42688_Gyro_FSR_t fsr)
{
    if (!pHandle)
        return false;

    if (((uint8_t)mode > 3U) || ((uint8_t)mode == 2U))
        return false;

    if ((((uint8_t)odr > (uint8_t)GYRO_ODR_500Hz)) || ((uint8_t)odr == 0x00U) || ((uint8_t)odr == 0x0CU) ||
        ((uint8_t)odr == 0x0DU) || ((uint8_t)odr == 0x0EU))
        return false;

    if ((uint8_t)fsr > (uint8_t)GYRO_FSR_15dps625)
        return false;

    bool _status = true;

    // (1) PWR_MGMT0: set gyro mode first because mode transitions have a required settling delay.
    ICM42688_Gyro_Mode_t _prev_mode       = pHandle->gyro_config.gyro_mode;
    ICM42688_Gyro_Mode_t _curr_mode       = mode;
    bool                 _need_write_mode = (!(pHandle->is_initialized) || (_curr_mode != _prev_mode));
    {
        // Skip the hardware write if the cached initialized state already matches the request.
        if (_need_write_mode == true) {
            _status = ICM42688_Update_Reg_Bits(pHandle, ICM42688_UB0_PWR_MGMT0, ICM42688_GYRO_MODE_Msk,
                                               ICM42688_GYRO_MODE_Val(mode));
            if (!_status)
                return false;

            pHandle->gyro_config.gyro_mode = mode;

            // Add delay >= 200 us when waking the gyro from OFF.
            if ((_prev_mode == GYRO_OFF) && (_curr_mode != GYRO_OFF)) {
                HAL_Delay(1); // 1 ms, safely above the 200 us minimum
            }
        }
    }

    // (2) GYRO_CONF0: set ODR and FSR together so cached scale factors match the register state.
    bool _need_write_conf = (!(pHandle->is_initialized) || (odr != (pHandle->gyro_config.gyro_odr)) ||
                             (fsr != (pHandle->gyro_config.gyro_fsr)));
    {
        if (_need_write_conf == true) {
            uint8_t _mask         = ICM42688_GYRO_ODR_Msk | ICM42688_GYRO_FS_SEL_Msk;
            uint8_t _value_masked = ICM42688_GYRO_ODR_Val(odr) | ICM42688_GYRO_FS_SEL_Val(fsr);

            _status = ICM42688_Update_Reg_Bits(pHandle, ICM42688_UB0_GYRO_CONF0, _mask, _value_masked);
            if (!_status)
                return false;

            // Update cache and scale factor only after the hardware write succeeds.
            pHandle->gyro_config.gyro_odr = odr;
            pHandle->gyro_config.gyro_fsr = fsr;

            ICM42688_Update_GyroScaleFactor(pHandle);
        }
    }
    return true;
}



bool
ICM42688_Get_Gyro_Mode(ICM42688_Handle_t *pHandle, uint8_t *pModeInfo)
{
    if (!pHandle || !pModeInfo)
        return false;

    uint8_t _reg    = 0U;
    bool    _status = ICM42688_ReadReg(pHandle, ICM42688_UB0_PWR_MGMT0, &_reg);
    if (!_status)
        return false;

    _reg &= ICM42688_GYRO_MODE_Msk;
    uint8_t _raw_mode = (uint8_t)(_reg >> ICM42688_GYRO_MODE_Pos);

    if ((_raw_mode == 0U) || (_raw_mode == 1U) || (_raw_mode == 3U)) {
        *pModeInfo                     = _raw_mode;
        pHandle->gyro_config.gyro_mode = (ICM42688_Gyro_Mode_t)_raw_mode;
        return true;
    }
    return false;
}



bool
ICM42688_Set_Gyro_UIFilt_BW(ICM42688_Handle_t *pHandle, ICM42688_UIFilt_BW_t uiFiltBandWidth)
{
    if (!pHandle)
        return false;

    if ((((uint8_t)uiFiltBandWidth >= 8U) && ((uint8_t)uiFiltBandWidth <= 13U)) ||
        (uint8_t)uiFiltBandWidth > 0x0FU)
        return false;

    bool _status =
        ICM42688_Update_Reg_Bits(pHandle, ICM42688_UB0_GYRO_ACCEL_CONF0, ICM42688_GYRO_UI_FILT_BW_Msk,
                                 ICM42688_GYRO_UI_FILT_BW_Val((uint8_t)uiFiltBandWidth));
    if (!_status)
        return false;

    pHandle->gyro_config.gyro_uifilt_bw = uiFiltBandWidth;

    return true;
}



bool
ICM42688_Set_Gyro_UIFilt_Order(ICM42688_Handle_t *pHandle, ICM42688_Gyro_UIFilt_Order_t uiFiltOrder)
{
    if (!pHandle)
        return false;

    if ((uint8_t)uiFiltOrder > (uint8_t)GYRO_THIRD_ORDER)
        return false;

    uint8_t _reg    = 0U;
    bool    _status = ICM42688_ReadReg(pHandle, ICM42688_UB0_GYRO_CONF1, &_reg);
    if (!_status)
        return false;

    _reg &= (uint8_t)~ICM42688_GYRO_UI_FILT_ORD_Msk;
    _reg |= (uint8_t)ICM42688_GYRO_UI_FILT_ORD_Val(uiFiltOrder);
    _status = ICM42688_WriteReg(pHandle, ICM42688_UB0_GYRO_CONF1, _reg);

    if (!_status)
        return false;

    pHandle->gyro_config.gyro_filt_order = uiFiltOrder;

    return true;
}



bool
ICM42688_Set_Gyro_Anti_Alias_Filt(ICM42688_Handle_t *pHandle, ICM42688_AAF_En_t antiAliasState)
{
    if (!pHandle)
        return false;

    if (((uint8_t)antiAliasState != 0U) && ((uint8_t)antiAliasState != 1U))
        return false;

    uint8_t _reg    = 0U;
    bool    _status = ICM42688_ReadReg(pHandle, ICM42688_UB1_GYRO_CONF_STATIC2, &_reg);
    if (!_status)
        return false;

    _reg &= (uint8_t)~ICM42688_GYRO_AAF_DIS_Msk;
    _reg |= (uint8_t)ICM42688_GYRO_AAF_DIS_Val(antiAliasState);
    _status = ICM42688_WriteReg(pHandle, ICM42688_UB1_GYRO_CONF_STATIC2, _reg);

    if (!_status)
        return false;

    pHandle->gyro_config.gyro_aaf_state = antiAliasState;

    return true;
}



bool
ICM42688_Set_Gyro_Notch_Filt(ICM42688_Handle_t *pHandle, ICM42688_Notch_Filt_En_t notchFiltState)
{
    if (!pHandle)
        return false;

    if (((uint8_t)notchFiltState != 0U) && ((uint8_t)notchFiltState != 1U))
        return false;

    uint8_t _reg    = 0U;
    bool    _status = ICM42688_ReadReg(pHandle, ICM42688_UB1_GYRO_CONF_STATIC2, &_reg);
    if (!_status)
        return false;

    _reg &= (uint8_t)~ICM42688_GYRO_NOTCH_FILT_Msk;
    _reg |= (uint8_t)ICM42688_GYRO_NOTCH_FILT_Val(notchFiltState);
    _status = ICM42688_WriteReg(pHandle, ICM42688_UB1_GYRO_CONF_STATIC2, _reg);

    if (!_status)
        return false;

    pHandle->gyro_config.gyro_notch_filt_state = notchFiltState;

    return true;
}



static bool
ICM42688_Compute_NotchFreq(uint16_t desiredNotchFreqHz, uint16_t *pNfCoswz, uint8_t *pNfCoswzSel)
{
    if (!pNfCoswz || !pNfCoswzSel)
        return false;

    if ((desiredNotchFreqHz < 1000U) || (desiredNotchFreqHz > 3000U))
        return false;

    float _desired_notch_freq_khz = (float)(desiredNotchFreqHz / 1000.0f);
    float _coswz                  = cosf(2.0f * M_PI * _desired_notch_freq_khz / 32.0f);

    if (fabsf(_coswz) <= 0.875f) {
        *pNfCoswz    = (uint16_t)lroundf(_coswz * 256.0f);
        *pNfCoswzSel = 0U;
        return true;
    }
    else {
        *pNfCoswzSel = 1U;
        if (_coswz > 0.875f) {
            *pNfCoswz = (uint16_t)lroundf(8.0f * (1.0f - _coswz) * 256.0f);
            return true;
        }
        else if (_coswz < -0.875f) {
            *pNfCoswz = (uint16_t)lroundf(-8.0f * (1.0f + _coswz) * 256.0f);
            return true;
        }
    }
    return false;
}



static bool
ICM42688_Set_NotchFreq_X(ICM42688_Handle_t *pHandle, uint16_t desiredNotchFreqHz)
{
    if (!pHandle)
        return false;

    // Convert requested notch frequency to the register representation shared by all gyro axes.
    uint16_t _nf_coswz_x     = 0U;
    uint8_t  _nf_coswz_x_sel = 0U;
    if (!ICM42688_Compute_NotchFreq(desiredNotchFreqHz, &_nf_coswz_x, &_nf_coswz_x_sel))
        return false;

    // Write nf_coswz_x[7:0] to the axis-specific low-byte register.
    uint8_t _reg_low  = 0U;
    uint8_t _reg_high = 0U;

    _reg_low     = (uint8_t)ICM42688_GYRO_X_NF_COSWZ_LOW_Val(_nf_coswz_x);
    bool _status = ICM42688_WriteReg(pHandle, ICM42688_UB1_GYRO_CONF_STATIC6, _reg_low);
    if (!_status)
        return false;

    // Write nf_coswz_x bit 8 and nf_coswz_x_sel to GYRO_CONF_STATIC9.
    _status = ICM42688_ReadReg(pHandle, ICM42688_UB1_GYRO_CONF_STATIC9, &_reg_high);
    if (!_status)
        return false;

    _reg_high &= (uint8_t)~(ICM42688_GYRO_X_NF_COSWZ_HIGH_Msk | ICM42688_GYRO_X_NF_COSWZ_SEL_Msk);
    _reg_high |= (uint8_t)(ICM42688_GYRO_X_NF_COSWZ_HIGH_Val((_nf_coswz_x >> 8U) & 0x01U) |
                           ICM42688_GYRO_X_NF_COSWZ_SEL_Val(_nf_coswz_x_sel));

    _status = ICM42688_WriteReg(pHandle, ICM42688_UB1_GYRO_CONF_STATIC9, _reg_high);
    if (!_status)
        return false;

    return true;
}



static bool
ICM42688_Set_NotchFreq_Y(ICM42688_Handle_t *pHandle, uint16_t desiredNotchFreqHz)
{
    if (!pHandle)
        return false;

    // Convert requested notch frequency to the register representation shared by all gyro axes.
    uint16_t _nf_coswz_y     = 0U;
    uint8_t  _nf_coswz_y_sel = 0U;
    if (!ICM42688_Compute_NotchFreq(desiredNotchFreqHz, &_nf_coswz_y, &_nf_coswz_y_sel))
        return false;

    // Write nf_coswz_y[7:0] to the axis-specific low-byte register.
    uint8_t _reg_low  = 0U;
    uint8_t _reg_high = 0U;

    _reg_low     = (uint8_t)ICM42688_GYRO_Y_NF_COSWZ_LOW_Val(_nf_coswz_y);
    bool _status = ICM42688_WriteReg(pHandle, ICM42688_UB1_GYRO_CONF_STATIC7, _reg_low);
    if (!_status)
        return false;

    // Write nf_coswz_y bit 8 and nf_coswz_y_sel to GYRO_CONF_STATIC9.
    _status = ICM42688_ReadReg(pHandle, ICM42688_UB1_GYRO_CONF_STATIC9, &_reg_high);
    if (!_status)
        return false;

    _reg_high &= (uint8_t)~(ICM42688_GYRO_Y_NF_COSWZ_HIGH_Msk | ICM42688_GYRO_Y_NF_COSWZ_SEL_Msk);
    _reg_high |= (uint8_t)(ICM42688_GYRO_Y_NF_COSWZ_HIGH_Val((_nf_coswz_y >> 8U) & 0x01U) |
                           ICM42688_GYRO_Y_NF_COSWZ_SEL_Val(_nf_coswz_y_sel));
    _status = ICM42688_WriteReg(pHandle, ICM42688_UB1_GYRO_CONF_STATIC9, _reg_high);
    if (!_status)
        return false;

    return true;
}



static bool
ICM42688_Set_NotchFreq_Z(ICM42688_Handle_t *pHandle, uint16_t desiredNotchFreqHz)
{
    if (!pHandle)
        return false;

    // Convert requested notch frequency to the register representation shared by all gyro axes.
    uint16_t _nf_coswz_z     = 0U;
    uint8_t  _nf_coswz_z_sel = 0U;
    if (!ICM42688_Compute_NotchFreq(desiredNotchFreqHz, &_nf_coswz_z, &_nf_coswz_z_sel))
        return false;

    // Write nf_coswz_z[7:0] to the axis-specific low-byte register.
    uint8_t _reg_low  = 0U;
    uint8_t _reg_high = 0U;

    _reg_low     = (uint8_t)ICM42688_GYRO_Z_NF_COSWZ_LOW_Val(_nf_coswz_z);
    bool _status = ICM42688_WriteReg(pHandle, ICM42688_UB1_GYRO_CONF_STATIC8, _reg_low);
    if (!_status)
        return false;

    // Write nf_coswz_z bit 8 and nf_coswz_z_sel to GYRO_CONF_STATIC9.
    _status = ICM42688_ReadReg(pHandle, ICM42688_UB1_GYRO_CONF_STATIC9, &_reg_high);
    if (!_status)
        return false;

    _reg_high &= (uint8_t)~(ICM42688_GYRO_Z_NF_COSWZ_HIGH_Msk | ICM42688_GYRO_Z_NF_COSWZ_SEL_Msk);
    _reg_high |= (uint8_t)(ICM42688_GYRO_Z_NF_COSWZ_HIGH_Val((_nf_coswz_z >> 8U) & 0x01U) |
                           ICM42688_GYRO_Z_NF_COSWZ_SEL_Val(_nf_coswz_z_sel));
    _status = ICM42688_WriteReg(pHandle, ICM42688_UB1_GYRO_CONF_STATIC9, _reg_high);
    if (!_status)
        return false;

    return true;
}


bool
ICM42688_Set_NotchFreq_XYZ(ICM42688_Handle_t *pHandle, uint16_t desiredXNotchFreqHz,
                           uint16_t desiredYNotchFreqHz, uint16_t desiredZNotchFreqHz)
{
    if (!pHandle)
        return false;

    if (!ICM42688_Set_NotchFreq_X(pHandle, desiredXNotchFreqHz))
        return false;

    if (!ICM42688_Set_NotchFreq_Y(pHandle, desiredYNotchFreqHz))
        return false;

    if (!ICM42688_Set_NotchFreq_Z(pHandle, desiredZNotchFreqHz))
        return false;

    return true;
}
