/*
 * icm42688_interface.c
 *
 *  Created on: Mar 12, 2026
 *      Author: dobao
 */
#include "imu/features/icm42688_interface.h"

/*=============================================================================
 *	SPI CONFIG
 *============================================================================ */
HAL_StatusTypeDef
ICM42688_Set_SPI_Mode(ICM42688_Handle_t *handle, ICM42688_SPI_Mode_t spiMode)
{
    if ((!handle) || ((spiMode != SPI_MODE_0_3) && (spiMode != SPI_MODE_1_2)))
        return HAL_ERROR;
    if (handle->is_initialized && (handle->spi_config.spi_mode == spiMode))
        return HAL_OK;

    HAL_StatusTypeDef status = ICM42688_Update_Reg_Bits(
        handle, ICM42688_UB0_DEVICE_CONF, ICM42688_DEVICE_CONFIG_SPI_MODE_Msk,
        ICM42688_DEVICE_CONFIG_SPI_MODE_Val(spiMode));
    if (status != HAL_OK)
        return status;

    handle->spi_config.spi_mode = spiMode;

    return HAL_OK;
}



HAL_StatusTypeDef
ICM42688_Get_SPI_SlewRate(ICM42688_Handle_t *handle, ICM42688_SPI_SLEWRATE_t *slewRate)
{
    if (!handle || !slewRate)
        return HAL_ERROR;

    uint8_t           reg    = 0U;
    HAL_StatusTypeDef status = ICM42688_ReadReg(handle, ICM42688_UB0_DRIVE_CONF, &reg);
    if (status != HAL_OK)
        return status;

    reg &= ICM42688_DRIVE_CONFIG_SPI_SR_Msk;
    *slewRate = (ICM42688_SPI_SLEWRATE_t)(reg >> ICM42688_DRIVE_CONFIG_SPI_SR_Pos);

    return status;
}



HAL_StatusTypeDef
ICM42688_Set_SPI_SlewRate(ICM42688_Handle_t *handle, ICM42688_SPI_SLEWRATE_t slewRate)
{
    if (!handle)
        return HAL_ERROR;

    if ((uint8_t)slewRate > 5U)
        return HAL_ERROR;

    if ((handle->is_initialized) && ((handle->spi_config.spi_slew_rate) == slewRate))
        return HAL_OK;

    HAL_StatusTypeDef status =
        ICM42688_Update_Reg_Bits(handle, ICM42688_UB0_DRIVE_CONF, ICM42688_DRIVE_CONFIG_SPI_SR_Msk,
                                 ICM42688_DRIVE_CONFIG_SPI_SR_Val(slewRate));
    if (status != HAL_OK)
        return status;

    handle->spi_config.spi_slew_rate = (ICM42688_SPI_SLEWRATE_t)slewRate;

    return HAL_OK;
}



HAL_StatusTypeDef
ICM42688_Set_UI_SIFS_Conf(ICM42688_Handle_t *handle, ICM42688_UI_SIFS_Cfg_t config)
{
    if (!handle)
        return HAL_ERROR;

    HAL_StatusTypeDef status =
        ICM42688_Update_Reg_Bits(handle, ICM42688_UB0_INTF_CONF0, ICM42688_UI_SIFS_CFG_Msk,
                                 ICM42688_UI_SIFS_CFG_Val(config));
    if (status != HAL_OK)
        return status;

    handle->intf_config.ui_sifs_config = config;
    return HAL_OK;
}



HAL_StatusTypeDef
ICM42688_Set_Sensor_Data_Endian(ICM42688_Handle_t            *handle,
                                ICM42688_Sensor_Data_Endian_t which_endian)
{
    if (!handle)
        return HAL_ERROR;

    HAL_StatusTypeDef status =
        ICM42688_Update_Reg_Bits(handle, ICM42688_UB0_INTF_CONF0, ICM42688_SENSOR_DATA_ENDIAN_Msk,
                                 ICM42688_SENSOR_DATA_ENDIAN_Val(which_endian));
    if (status != HAL_OK)
        return status;

    handle->intf_config.sensor_data_endian = which_endian;

    return HAL_OK;
}
