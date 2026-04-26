/*
 * icm42688_temp.c
 *
 *  Created on: Mar 12, 2026
 *      Author: dobao
 */

#include "imu/sensors/icm42688_temp.h"

/*==================================================================================
 * 	TEMPERATURE CONFIG
 *================================================================================== */
ICM42688_Status_t
ICM42688_Set_Temperature_Enable(ICM42688_Handle_t *handle, ICM42688_Temp_t state)
{
    if (!handle)
        return HAL_ERROR;

    if (handle->is_initialized && handle->temp_config.temp_state == state)
        return HAL_OK;
    HAL_StatusTypeDef status = ICM42688_Update_Reg_Bits(
        handle, ICM42688_UB0_PWR_MGMT0, ICM42688_TEMP_Msk, ICM42688_TEMP_Val(state));
    if (status != HAL_OK)
        return status;
    handle->temp_config.temp_state = state;
    return HAL_OK;
}
