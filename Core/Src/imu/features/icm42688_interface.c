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
 * @param   handle      Pointer to ICM42688 handle struct
 * @param   spiMode     SPI_MODE_0_3 or SPI_MODE_1_2
 */
ICM42688_Status_t
ICM42688_Set_SPI_Mode(ICM42688_Handle_t *handle, ICM42688_SPI_Mode_t spiMode)
{
    if ((!handle) || ((spiMode != SPI_MODE_0_3) && (spiMode != SPI_MODE_1_2)))
        return ICM42688_ERROR;

    if (handle->is_initialized && (handle->spi_config.spi_mode == spiMode))
        return ICM42688_OK;

    HAL_StatusTypeDef status = ICM42688_Update_Reg_Bits(
        handle, ICM42688_UB0_DEVICE_CONF, ICM42688_DEVICE_CONFIG_SPI_MODE_Msk,
        ICM42688_DEVICE_CONFIG_SPI_MODE_Val(spiMode));
    if (status != HAL_OK)
        return ICM42688_ERROR;

    handle->spi_config.spi_mode = spiMode;

    return ICM42688_OK;
}



ICM42688_Status_t
ICM42688_Get_SPI_SlewRate(ICM42688_Handle_t *handle, ICM42688_SPI_SLEWRATE_t *slewRate)
{
    if (!handle || !slewRate)
        return ICM42688_ERROR;

    uint8_t           reg    = 0U;
    HAL_StatusTypeDef status = ICM42688_ReadReg(handle, ICM42688_UB0_DRIVE_CONF, &reg);
    if (status != HAL_OK)
        return ICM42688_ERROR;

    reg &= ICM42688_DRIVE_CONFIG_SPI_SR_Msk;
    *slewRate = (ICM42688_SPI_SLEWRATE_t)(reg >> ICM42688_DRIVE_CONFIG_SPI_SR_Pos);

    return ICM42688_OK;
}



ICM42688_Status_t
ICM42688_Set_SPI_SlewRate(ICM42688_Handle_t *handle, ICM42688_SPI_SLEWRATE_t slewRate)
{
    if (!handle)
        return ICM42688_ERROR;

    if ((uint8_t)slewRate > 5U)
        return ICM42688_ERROR;

    if ((handle->is_initialized) && ((handle->spi_config.spi_slew_rate) == slewRate))
        return ICM42688_OK;

    HAL_StatusTypeDef status =
        ICM42688_Update_Reg_Bits(handle, ICM42688_UB0_DRIVE_CONF, ICM42688_DRIVE_CONFIG_SPI_SR_Msk,
                                 ICM42688_DRIVE_CONFIG_SPI_SR_Val(slewRate));
    if (status != HAL_OK)
        return ICM42688_ERROR;

    handle->spi_config.spi_slew_rate = (ICM42688_SPI_SLEWRATE_t)slewRate;

    return ICM42688_OK;
}



ICM42688_Status_t
ICM42688_Set_UI_SIFS_Conf(ICM42688_Handle_t *handle, ICM42688_UI_SIFS_Cfg_t UI_SIFS_Config)
{
    if (!handle)
        return ICM42688_ERROR;

    HAL_StatusTypeDef status =
        ICM42688_Update_Reg_Bits(handle, ICM42688_UB0_INTF_CONF0, ICM42688_UI_SIFS_CFG_Msk,
                                 ICM42688_UI_SIFS_CFG_Val(UI_SIFS_Config));
    if (status != HAL_OK)
        return ICM42688_ERROR;

    handle->intf_config.ui_sifs_config = UI_SIFS_Config;

    return ICM42688_OK;
}



ICM42688_Status_t
ICM42688_Set_Sensor_Data_Endian(ICM42688_Handle_t            *handle,
                                ICM42688_Sensor_Data_Endian_t whichEndian)
{
    if (!handle)
        return ICM42688_ERROR;

    HAL_StatusTypeDef status =
        ICM42688_Update_Reg_Bits(handle, ICM42688_UB0_INTF_CONF0, ICM42688_SENSOR_DATA_ENDIAN_Msk,
                                 ICM42688_SENSOR_DATA_ENDIAN_Val(whichEndian));
    if (status != HAL_OK)
        return ICM42688_ERROR;

    handle->intf_config.sensor_data_endian = whichEndian;

    return ICM42688_OK;
}
