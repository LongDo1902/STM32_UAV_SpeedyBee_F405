/*
 * icm42688_sensor.c
 *
 *  Created on: Mar 12, 2026
 *      Author: dobao
 */
#include "imu/sensors/icm42688_gyro.h"

static const float lsb_per_dps[] = {
    16.4f,
    32.8f,
    65.5f,
    131.0f,
    262.0f,
    524.3f,
    1048.6f,
    2097.2f
};

static inline void ICM42688_Update_GyroScaleFactor(ICM42688_Handle_t *handle)
{
    uint8_t idx = (uint8_t)handle->gyro_config.gyro_fsr;
    if (idx > 7U) idx = 0U;

    handle->gyro_lsb_per_dps_dtsheet = lsb_per_dps[idx];
    handle->gyro_dps_per_lsb = 1.0f / handle->gyro_lsb_per_dps_dtsheet;
}

/*=============================================================================
 *	GYRO CONFIG / FILTER
 *============================================================================= */
HAL_StatusTypeDef ICM42688_Set_GyroConfig(ICM42688_Handle_t* handle, ICM42688_GyroMode_t mode,
										  ICM42688_GyroODR_t odr, ICM42688_GyroFSR_t fsr)
{
	if(!handle) return HAL_ERROR;

	if(((uint8_t)mode > 3U) || ((uint8_t)mode == 2U)) return HAL_ERROR;

	if((((uint8_t)odr > (uint8_t)GYRO_ODR_500Hz)) ||
		((uint8_t)odr == 0x00U) ||
		((uint8_t)odr == 0x0CU) ||
		((uint8_t)odr == 0x0DU) ||
		((uint8_t)odr == 0x0EU)) return HAL_ERROR;

	if((uint8_t)fsr > (uint8_t)GYRO_FSR_15dps625) return HAL_ERROR;

	HAL_StatusTypeDef status = HAL_OK;


	// (1) PWR_MGMT0: Set Gyro Mode
	ICM42688_GyroMode_t prevMode = handle -> gyro_config.gyro_mode;
	ICM42688_GyroMode_t currMode = mode;
	bool need_write_mode = (!(handle -> is_initialized) || (currMode != prevMode));
	{
		//Skip if already cached & initialized
		if(need_write_mode == true){
			status = ICM42688_Update_Reg_Bits(handle,
											ICM42688_UB0_PWR_MGMT0,
											ICM42688_GYRO_MODE_Msk,
											ICM42688_GYRO_MODE_Val(mode));
			if(status != HAL_OK) return status;

			handle -> gyro_config.gyro_mode = mode;

			//Add delay >= 200us according to datasheet
			if((prevMode == GYRO_OFF) && (currMode != GYRO_OFF)){
				HAL_Delay(1); //1000us
			}
		}
	}


	// (2) GYRO_CONF0: Set ODR + FSR together
	bool need_write_conf = (!(handle -> is_initialized) ||
						(odr != (handle -> gyro_config.gyro_odr)) ||
						(fsr != (handle -> gyro_config.gyro_fsr)));
	{
		if(need_write_conf == true){
			uint8_t mask 		= ICM42688_GYRO_ODR_Msk | ICM42688_GYRO_FS_SEL_Msk;
			uint8_t valueMasked	= ICM42688_GYRO_ODR_Val(odr) | ICM42688_GYRO_FS_SEL_Val(fsr);
			status = ICM42688_Update_Reg_Bits(handle,
											ICM42688_UB0_GYRO_CONF0,
											mask,
											valueMasked);
			if(status != HAL_OK) return status;

			//Update cache + scale factor after successful HW write
			handle -> gyro_config.gyro_odr = odr;
			handle -> gyro_config.gyro_fsr = fsr;
			ICM42688_Update_GyroScaleFactor(handle);
		}
	}
	return status;
}


HAL_StatusTypeDef ICM42688_Get_Gyro_Mode(ICM42688_Handle_t* handle, uint8_t* modeInfo)
{
	if(!handle || !modeInfo) return HAL_ERROR;

	uint8_t reg = 0U;
	HAL_StatusTypeDef status = ICM42688_ReadReg(handle, ICM42688_UB0_PWR_MGMT0, &reg);
	if(status != HAL_OK) return status;

	reg &= ICM42688_GYRO_MODE_Msk;
	uint8_t rawMode = (uint8_t)(reg >> ICM42688_GYRO_MODE_Pos);

	if((rawMode == 0U) || (rawMode == 1U) || (rawMode == 3U)){
		*modeInfo = rawMode;
		handle -> gyro_config.gyro_mode = (ICM42688_GyroMode_t)rawMode;
		return HAL_OK;
	}
	return HAL_ERROR;
}


HAL_StatusTypeDef ICM42688_Set_Gyro_UIFilt_BW(ICM42688_Handle_t* handle, ICM42688_UIFilt_BW_t bw)
{
	if(!handle) return HAL_ERROR;
	if((((uint8_t)bw >= 8U) && ((uint8_t)bw <= 13U)) || (uint8_t)bw > 0x0FU) return HAL_ERROR;

	HAL_StatusTypeDef status = ICM42688_Update_Reg_Bits(handle,
														ICM42688_UB0_GYRO_ACCEL_CONF0,
														ICM42688_GYRO_UI_FILT_BW_Msk,
														ICM42688_GYRO_UI_FILT_BW_Val((uint8_t)bw));
	if(status != HAL_OK) return status;
	handle -> gyro_config.gyro_uifilt_bw = bw;
	return HAL_OK;
}
