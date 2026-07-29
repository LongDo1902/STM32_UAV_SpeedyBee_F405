/*
 * icm42688_interface.c
 *
 *  Created on: Mar 12, 2026
 *      Author: dobao
 */
#include "imu/features/icm42688_interface.h"


/*=============================================================================
 *	SPI CONFIG
 * ============================================================================= */
/**
 * @brief   Write a desired SPI mode
 * @param   pHandle     Pointer to ICM42688 handle struct
 * @param   spiMode     SPI_MODE_0_3 or SPI_MODE_1_2
 */
bool
ICM42688_Set_SPI_Mode(ICM42688_Handle_t *pHandle, ICM42688_SPI_Mode_t spiMode)
{
    if ((!pHandle) || ((spiMode != SPI_MODE_0_3) && (spiMode != SPI_MODE_1_2)))
        return false;

    if (pHandle->is_initialized && (pHandle->spi_config.spi_mode == spiMode))
        return true;

    bool _status = ICM42688_Update_Reg_Bits(pHandle, ICM42688_UB0_DEVICE_CONF, ICM42688_DEVICE_CONFIG_SPI_MODE_Msk,
                                            ICM42688_DEVICE_CONFIG_SPI_MODE_Val(spiMode));
    if (!_status)
        return false;

    pHandle->spi_config.spi_mode = spiMode;

    return true;
}



bool
ICM42688_Get_SPI_SlewRate(ICM42688_Handle_t *pHandle, ICM42688_SPI_SLEWRATE_t *pSlewRate)
{
    if (!pHandle || !pSlewRate)
        return false;

    uint8_t _reg    = 0U;
    bool    _status = ICM42688_ReadReg(pHandle, ICM42688_UB0_DRIVE_CONF, &_reg);
    if (!_status)
        return false;

    _reg &= ICM42688_DRIVE_CONFIG_SPI_SR_Msk;
    *pSlewRate = (ICM42688_SPI_SLEWRATE_t)(_reg >> ICM42688_DRIVE_CONFIG_SPI_SR_Pos);

    return true;
}



bool
ICM42688_Set_SPI_SlewRate(ICM42688_Handle_t *pHandle, ICM42688_SPI_SLEWRATE_t slewRate)
{
    if (!pHandle)
        return false;

    if ((uint8_t)slewRate > 5U)
        return false;

    if ((pHandle->is_initialized) && ((pHandle->spi_config.spi_slew_rate) == slewRate))
        return true;

    bool _status = ICM42688_Update_Reg_Bits(pHandle, ICM42688_UB0_DRIVE_CONF, ICM42688_DRIVE_CONFIG_SPI_SR_Msk,
                                            ICM42688_DRIVE_CONFIG_SPI_SR_Val(slewRate));
    if (!_status)
        return false;

    pHandle->spi_config.spi_slew_rate = (ICM42688_SPI_SLEWRATE_t)slewRate;

    return true;
}



bool
ICM42688_Set_UI_SIFS_Conf(ICM42688_Handle_t *pHandle, ICM42688_UI_SIFS_Cfg_t uiSifsConfig)
{
    if (!pHandle)
        return false;

    if ((uiSifsConfig != UI_SIFS_RESERVED) && (uiSifsConfig != UI_SIFS_DISABLE_SPI) &&
        (uiSifsConfig != UI_SIFS_DISABLE_I2C))
        return false;

    bool _status = ICM42688_Update_Reg_Bits(pHandle, ICM42688_UB0_INTF_CONF0, ICM42688_UI_SIFS_CFG_Msk,
                                            ICM42688_UI_SIFS_CFG_Val(uiSifsConfig));
    if (!_status)
        return false;

    pHandle->intf_config.ui_sifs_config = uiSifsConfig;

    return true;
}



bool
ICM42688_Set_Sensor_Data_Endian(ICM42688_Handle_t *pHandle, ICM42688_Sensor_Data_Endian_t whichEndian)
{
    if (!pHandle)
        return false;

    if (((uint8_t)whichEndian != 0U) && ((uint8_t)whichEndian != 1U))
        return false;

    bool _status = ICM42688_Update_Reg_Bits(pHandle, ICM42688_UB0_INTF_CONF0, ICM42688_SENSOR_DATA_ENDIAN_Msk,
                                            ICM42688_SENSOR_DATA_ENDIAN_Val(whichEndian));
    if (!_status)
        return false;

    pHandle->intf_config.sensor_data_endian = whichEndian;

    return true;
}
