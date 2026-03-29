/*
 * icm42688_interrupt.c
 *
 *  Created on: Mar 12, 2026
 *      Author: dobao
 */

#include "imu/features/icm42688_interrupt.h"

/*==================================================================================
 *	INTERRUPT CONFIG
 *================================================================================== */
HAL_StatusTypeDef
ICM42688_Set_Int1_Config(ICM42688_Handle_t *handle, ICM42688_Int_Polarity_t polarity,
                         ICM42688_Int_Drive_Circuit_t drive, ICM42688_Int_Mode_t mode)
{
    if ((!handle) || ((uint8_t)polarity >= INT_POL_MAX) || ((uint8_t)drive >= INT_DRIVE_MAX) ||
        ((uint8_t)mode >= INT_MODE_MAX))
        return HAL_ERROR;

    if ((handle->is_initialized) &&
        ((uint8_t)polarity == (uint8_t)(handle->int1_config.int1_polarity)) &&
        ((uint8_t)drive) == ((uint8_t)(handle->int1_config.int1_drive)) &&
        ((uint8_t)mode) == ((uint8_t)(handle->int1_config.int1_mode))) {
        return HAL_OK;
    }

    uint8_t mask        = ICM42688_INT1_POL_Msk | ICM42688_INT1_DRIVE_Msk | ICM42688_INT1_MODE_Msk;
    uint8_t valueMasked = ICM42688_INT1_POL_Val(polarity) | ICM42688_INT1_DRIVE_Val(drive) |
                          ICM42688_INT1_MODE_Val(mode);
    HAL_StatusTypeDef status =
        ICM42688_Update_Reg_Bits(handle, ICM42688_UB0_INT_CONF, mask, valueMasked);
    if (status != HAL_OK)
        return status;

    handle->int1_config.int1_polarity = polarity;
    handle->int1_config.int1_drive    = drive;
    handle->int1_config.int1_mode     = mode;

    return HAL_OK;
}



HAL_StatusTypeDef
ICM42688_Set_Int2_Config(ICM42688_Handle_t *handle, ICM42688_Int_Polarity_t polarity,
                         ICM42688_Int_Drive_Circuit_t drive, ICM42688_Int_Mode_t mode)
{
    if ((!handle) || ((uint8_t)polarity >= INT_POL_MAX) || ((uint8_t)drive >= INT_DRIVE_MAX) ||
        ((uint8_t)mode >= INT_MODE_MAX))
        return HAL_ERROR;

    if ((handle->is_initialized) &&
        ((uint8_t)polarity == (uint8_t)(handle->int2_config.int2_polarity)) &&
        ((uint8_t)drive) == ((uint8_t)(handle->int2_config.int2_drive)) &&
        ((uint8_t)mode) == ((uint8_t)(handle->int2_config.int2_mode)))
        return HAL_OK;

    uint8_t mask        = ICM42688_INT2_POL_Msk | ICM42688_INT2_DRIVE_Msk | ICM42688_INT2_MODE_Msk;
    uint8_t valueMasked = ICM42688_INT2_POL_Val(polarity) | ICM42688_INT2_DRIVE_Val(drive) |
                          ICM42688_INT2_MODE_Val(mode);

    HAL_StatusTypeDef status =
        ICM42688_Update_Reg_Bits(handle, ICM42688_UB0_INT_CONF, mask, valueMasked);
    if (status != HAL_OK)
        return status;

    handle->int2_config.int2_polarity = polarity;
    handle->int2_config.int2_drive    = drive;
    handle->int2_config.int2_mode     = mode;

    return HAL_OK;
}



HAL_StatusTypeDef
ICM42688_Get_Int_Status(ICM42688_Handle_t *handle, uint8_t *outStatus)
{
    if (!handle || !outStatus)
        return HAL_ERROR;

    uint8_t           reg    = 0U;
    HAL_StatusTypeDef status = ICM42688_ReadReg(handle, ICM42688_UB0_INT_STATUS, &reg);
    if (status != HAL_OK)
        return status;

    *outStatus = reg;
    return HAL_OK;
}



/*==========================================================================================
 * 	INT_STATUS
 *==========================================================================================*/
bool
ICM42688_Int_Status_Has(uint8_t status, ICM42688_Int_Status_t intState)
{
    uint8_t mask = 0U;

    switch (intState) {
    case INT_AGC_RDY:
        mask = (uint8_t)ICM42688_AGC_RDY_INT_Msk;
        break;
    case INT_FIFO_FULL:
        mask = (uint8_t)ICM42688_FIFO_FULL_INT_Msk;
        break;
    case INT_FIFO_THS:
        mask = (uint8_t)ICM42688_FIFO_THS_INT_Msk;
        break;
    case INT_DATA_RDY:
        mask = (uint8_t)ICM42688_DATA_RDY_INT_Msk;
        break;
    case INT_RESET_DONE:
        mask = (uint8_t)ICM42688_RESET_DONE_INT_Msk;
        break;
    case INT_PLL_RDY:
        mask = (uint8_t)ICM42688_PLL_RDY_INT_Msk;
        break;
    case INT_UI_FSYNC:
        mask = (uint8_t)ICM42688_UI_FSYNC_INT_Msk;
        break;
    default:
        return false;
    }

    return (((status & 0x7FU) & mask) != 0U);
}



/*==========================================================================================
 * 	INT_SOURCE0
 *==========================================================================================*/
HAL_StatusTypeDef
ICM42688_Set_Int1_FIFO_Full_Enable(ICM42688_Handle_t *handle, bool enable)
{
    if (!handle)
        return HAL_ERROR;
    uint8_t           valueMasked = enable ? ICM42688_FIFO_FULL_INT1_EN_Msk : 0U;
    HAL_StatusTypeDef status      = ICM42688_Update_Reg_Bits(
        handle, ICM42688_UB0_INT_SRC0, ICM42688_FIFO_FULL_INT1_EN_Msk, valueMasked);
    if (status != HAL_OK)
        return status;
    return HAL_OK;
}



HAL_StatusTypeDef
ICM42688_Set_Int1_FIFO_Threshold_Enable(ICM42688_Handle_t *handle, bool enable)
{
    if (!handle)
        return HAL_ERROR;
    uint8_t           valueMasked = enable ? ICM42688_FIFO_THS_INT1_EN_Msk : 0U;
    HAL_StatusTypeDef status = ICM42688_Update_Reg_Bits(handle, ICM42688_UB0_INT_SRC0,
                                                        ICM42688_FIFO_THS_INT1_EN_Msk, valueMasked);
    if (status != HAL_OK)
        return status;
    return HAL_OK;
}



HAL_StatusTypeDef
ICM42688_Set_Int1_DataReady_Enable(ICM42688_Handle_t *handle, bool enable)
{
    if (!handle)
        return HAL_ERROR;
    uint8_t           valueMasked = enable ? ICM42688_UI_DRDY_INT1_EN_Msk : 0U;
    HAL_StatusTypeDef status = ICM42688_Update_Reg_Bits(handle, ICM42688_UB0_INT_SRC0,
                                                        ICM42688_UI_DRDY_INT1_EN_Msk, valueMasked);
    if (status != HAL_OK)
        return status;
    return HAL_OK;
}



HAL_StatusTypeDef
ICM42688_Set_Int1_ResetDone_Enable(ICM42688_Handle_t *handle, bool enable)
{
    if (!handle)
        return HAL_ERROR;
    uint8_t           valueMasked = enable ? ICM42688_RESET_DONE_INT1_EN_Msk : 0U;
    HAL_StatusTypeDef status      = ICM42688_Update_Reg_Bits(
        handle, ICM42688_UB0_INT_SRC0, ICM42688_RESET_DONE_INT1_EN_Msk, valueMasked);
    if (status != HAL_OK)
        return status;
    return HAL_OK;
}
