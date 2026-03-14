/*
 * icm42688_fifo.c
 *
 *  Created on: Mar 12, 2026
 *      Author: dobao
 */

#include "imu/features/icm42688_fifo.h"

/*==================================================================================
 *	FIFO CONFIG
 *================================================================================== */
HAL_StatusTypeDef ICM42688_Set_FIFO_Count_Endian(ICM42688_Handle_t* handle, ICM42688_FIFO_Count_Endian_t which_endian)
{
	if(!handle) return HAL_ERROR;
	if(((uint8_t)which_endian != 0U && (uint8_t)which_endian != 1U)) return HAL_ERROR;

	HAL_StatusTypeDef status = ICM42688_Update_Reg_Bits(handle,
														ICM42688_UB0_INTF_CONF0,
														ICM42688_FIFO_COUNT_ENDIAN_Msk,
														ICM42688_FIFO_COUNT_ENDIAN_Val(which_endian));
	if(status != HAL_OK) return status;
	handle -> fifo_config.fifo_count_endian = which_endian;
	return HAL_OK;
}


HAL_StatusTypeDef ICM42688_Set_FIFO_Count_Rec(ICM42688_Handle_t* handle, ICM42688_FIFO_Count_Rec_t fifo_count_rec)
{
	if(!handle) return HAL_ERROR;

	HAL_StatusTypeDef status = ICM42688_Update_Reg_Bits(handle,
														ICM42688_UB0_INTF_CONF0,
														ICM42688_FIFO_COUNT_REC_Msk,
														ICM42688_FIFO_COUNT_REC_Val(fifo_count_rec));
	if(status != HAL_OK) return status;
	handle -> fifo_config.fifo_count_rec = fifo_count_rec;
	return HAL_OK;
}


HAL_StatusTypeDef ICM42688_Set_FIFO_Mode(ICM42688_Handle_t* handle, ICM42688_FIFO_Mode_t mode)
{
	if(!handle) return HAL_ERROR;
	if((uint8_t)mode > (uint8_t)STOP_ON_FULL) return HAL_ERROR;

	HAL_StatusTypeDef status = ICM42688_Update_Reg_Bits(handle,
														ICM42688_UB0_FIFO_CONF,
														ICM42688_FIFO_MODE_Msk,
														ICM42688_FIFO_MODE_Val(mode));
	if(status != HAL_OK) return status;
	handle -> fifo_config.fifo_mode = (ICM42688_FIFO_Mode_t)mode;
	return HAL_OK;
}


HAL_StatusTypeDef ICM42688_Get_FIFO_Mode(ICM42688_Handle_t* handle, ICM42688_FIFO_Mode_t* mode)
{
	if(!handle || !mode) return HAL_ERROR;

	uint8_t reg = 0U;
	HAL_StatusTypeDef status = ICM42688_ReadReg(handle, ICM42688_UB0_FIFO_CONF, &reg);
	if(status != HAL_OK) return status;

	uint8_t rawMode = (uint8_t)((reg & ICM42688_FIFO_MODE_Msk) >> ICM42688_FIFO_MODE_Pos);
	bool isModeStopOnFull = (rawMode == 2U) || (rawMode == 3U);
	*mode = (isModeStopOnFull) ? STOP_ON_FULL : (ICM42688_FIFO_Mode_t)rawMode;

	handle -> fifo_config.fifo_mode = (ICM42688_FIFO_Mode_t)*mode;
	return HAL_OK;
}


HAL_StatusTypeDef ICM42688_Set_FIFO_Gyro_Enable(ICM42688_Handle_t* handle, ICM42688_FIFO_GAT_En_t state)
{
	if(!handle) return HAL_ERROR;

	HAL_StatusTypeDef status = ICM42688_Update_Reg_Bits(handle,
														ICM42688_UB0_FIFO_CONF1,
														ICM42688_FIFO_GYRO_EN_Msk,
														ICM42688_FIFO_GYRO_EN_Val(state));
	if(status != HAL_OK) return status;
	handle -> fifo_config.fifo_gyro_state = (ICM42688_FIFO_GAT_En_t)state;
	return HAL_OK;
}


HAL_StatusTypeDef ICM42688_Set_FIFO_Accel_Enable(ICM42688_Handle_t* handle, ICM42688_FIFO_GAT_En_t state)
{
	if(!handle) return HAL_ERROR;

	HAL_StatusTypeDef status = ICM42688_Update_Reg_Bits(handle,
														ICM42688_UB0_FIFO_CONF1,
														ICM42688_FIFO_ACCEL_EN_Msk,
														ICM42688_FIFO_ACCEL_EN_Val(state));
	if(status != HAL_OK) return status;
	handle -> fifo_config.fifo_accel_state = (ICM42688_FIFO_GAT_En_t)state;
	return HAL_OK;
}


HAL_StatusTypeDef ICM42688_Set_FIFO_Temp_Enable(ICM42688_Handle_t* handle, ICM42688_FIFO_GAT_En_t state)
{
	if(!handle) return HAL_ERROR;

	HAL_StatusTypeDef status = ICM42688_Update_Reg_Bits(handle,
														ICM42688_UB0_FIFO_CONF1,
														ICM42688_FIFO_TEMP_EN_Msk,
														ICM42688_FIFO_TEMP_EN_Val(state));
	if(status != HAL_OK) return status;
	handle -> fifo_config.fifo_temp_state = (ICM42688_FIFO_GAT_En_t)state;
	return HAL_OK;
}


HAL_StatusTypeDef ICM42688_Set_FIFO_HIRES_Enable(ICM42688_Handle_t* handle, ICM42688_FIFO_Hires_En_t state)
{
	if(!handle) return HAL_ERROR;

	HAL_StatusTypeDef status = ICM42688_Update_Reg_Bits(handle,
														ICM42688_UB0_FIFO_CONF1,
														ICM42688_FIFO_HIRES_EN_Msk,
														ICM42688_FIFO_HIRES_EN_Val(state));
	if(status != HAL_OK) return status;
	handle -> fifo_config.fifo_hires_state = (ICM42688_FIFO_Hires_En_t)state;
	return HAL_OK;
}


HAL_StatusTypeDef ICM42688_Set_FIFO_WM_GT_THS(ICM42688_Handle_t* handle, ICM42688_FIFO_WM_Mode_t state)
{
	if(!handle) return HAL_ERROR;

	HAL_StatusTypeDef status = ICM42688_Update_Reg_Bits(handle,
														ICM42688_UB0_FIFO_CONF1,
														ICM42688_FIFO_WM_GT_TH_Msk,
														ICM42688_FIFO_WM_GT_TH_Val(state));
	if(status != HAL_OK) return status;
	handle -> fifo_config.fifo_wm_mode = (ICM42688_FIFO_WM_Mode_t) state;
	return HAL_OK;
}


HAL_StatusTypeDef ICM42688_Set_FIFO_Resume_Partial_Read(ICM42688_Handle_t* handle, ICM42688_FIFO_Resume_Read_t state)
{
	if(!handle) return HAL_ERROR;
	HAL_StatusTypeDef status = ICM42688_Update_Reg_Bits(handle,
														ICM42688_UB0_FIFO_CONF1,
														ICM42688_FIFO_RESUME_PARTIAL_RD_Msk,
														ICM42688_FIFO_RESUME_PARTIAL_RD_Val(state));
	if(status != HAL_OK) return status;
	handle -> fifo_config.fifo_partial_read_state = (ICM42688_FIFO_Resume_Read_t) state;
	return HAL_OK;
}


HAL_StatusTypeDef ICM42688_Set_FIFO_Watermark(ICM42688_Handle_t* handle, uint16_t fifoWatermark)
{
	if(!handle) return HAL_ERROR;
	if((fifoWatermark == 0U) || (fifoWatermark > 0x0FFFU)) return HAL_ERROR;

	uint8_t fifoLowerWm = (uint8_t)(fifoWatermark & 0x00FFU);
	uint8_t fifoUpperWm = (uint8_t)((fifoWatermark >> 8) & 0x0FU);

	HAL_StatusTypeDef status = ICM42688_Update_Reg_Bits(handle,
														ICM42688_UB0_FIFO_CONF2,
														ICM42688_FIFO_WM_LOWER_Msk,
														ICM42688_FIFO_WM_LOWER_Val(fifoLowerWm));
	if(status != HAL_OK) return status;

	status = ICM42688_Update_Reg_Bits(handle,
									ICM42688_UB0_FIFO_CONF3,
									ICM42688_FIFO_WM_UPPER_Msk,
									ICM42688_FIFO_WM_UPPER_Val(fifoUpperWm));
	if(status != HAL_OK) return status;

	handle -> fifo_config.fifo_watermark = (uint16_t)fifoWatermark;
	return HAL_OK;
}


HAL_StatusTypeDef ICM42688_Get_FIFO_Watermark(ICM42688_Handle_t* handle, uint16_t* fifoWatermark)
{
	if(!handle || !fifoWatermark) return HAL_ERROR;

	uint8_t fifoWmBuf[2];
	HAL_StatusTypeDef status = ICM42688_ReadRegs(handle, ICM42688_UB0_FIFO_CONF2, fifoWmBuf, 2);
	if(status != HAL_OK) return status;

	*fifoWatermark = (uint16_t)((uint16_t)((fifoWmBuf[1] & ICM42688_FIFO_WM_UPPER_Msk) << 8U) |
								(uint16_t)((fifoWmBuf[0] & ICM42688_FIFO_WM_LOWER_Msk)));

	handle -> fifo_config.fifo_watermark = (uint16_t)*fifoWatermark;
	return HAL_OK;
}


HAL_StatusTypeDef ICM42688_Get_FIFO_Count(ICM42688_Handle_t* handle, uint16_t* fifoCount)
{
	if(!handle || !fifoCount) return HAL_ERROR;

	uint8_t fifoCountBuf[2];
	HAL_StatusTypeDef status = ICM42688_ReadRegs(handle, ICM42688_UB0_FIFO_COUNTH, fifoCountBuf, 2);
	if(status != HAL_OK) return status;

	if(handle -> fifo_config.fifo_count_endian == FIFO_COUNT_BIG_ENDIAN){ //Big endian format
		*fifoCount = (uint16_t)(((uint16_t)(fifoCountBuf[0] & 0xFFU) << 8) |
								((uint16_t)(fifoCountBuf[1] & 0xFFU)));
	}
	else{
		*fifoCount = (uint16_t)(((uint16_t)(fifoCountBuf[1] & 0xFFU) << 8) |
								((uint16_t)(fifoCountBuf[0] & 0xFFU)));
	}

	handle -> fifo_config.fifo_count = (uint16_t)*fifoCount;
	return HAL_OK;
}


HAL_StatusTypeDef ICM42688_Get_FIFO_Data(ICM42688_Handle_t* handle, uint8_t* buf,
										uint16_t bufSize, uint16_t* byteRead)
{
	if(!handle || !buf || !byteRead) return HAL_ERROR;

	*byteRead = 0U;

	uint16_t fifoCount	= 0U;
	uint16_t recordSize = 0U;
	uint16_t totalByte	= 0U;

	HAL_StatusTypeDef status = ICM42688_Get_FIFO_Count(handle, &fifoCount);
	if(status != HAL_OK) return status;

	if(handle -> fifo_config.fifo_count_rec == FIFO_COUNT_IN_RECORD){
		if(handle -> fifo_config.fifo_hires_state){
			recordSize = 20U;
		}
		else if((handle -> fifo_config.fifo_accel_state == FIFO_GAT_ENABLE) &&
				(handle -> fifo_config.fifo_gyro_state == FIFO_GAT_ENABLE)){
			recordSize = 16U;
		}
		else if ((handle -> fifo_config.fifo_accel_state == FIFO_GAT_ENABLE) ||
				(handle -> fifo_config.fifo_gyro_state == FIFO_GAT_ENABLE)){
			recordSize = 8U;
		}
		else return HAL_ERROR;

		totalByte = (uint16_t)(fifoCount * recordSize);
	}

	else{
		totalByte = fifoCount;
	}

	if(totalByte == 0U) return HAL_OK;

	/* Whole logic below is:
	 * 		If totalByte fits user buffer, read all
	 * 		If it does not fits, partial read is disabled, return error
	 * 		If it does not fits, partial read is enabled, read only what fits */
	if(totalByte > bufSize){
		if(handle -> fifo_config.fifo_partial_read_state == FIFO_PARTIAL_READ_DISABLE){
			return HAL_ERROR;
		}

		//Partial-read enabled: only read what fit
		if(handle -> fifo_config.fifo_count_rec == FIFO_COUNT_IN_RECORD){
			if((recordSize == 0) || (bufSize < recordSize)) return HAL_ERROR;
			totalByte = (uint16_t)((bufSize / recordSize) * recordSize);
		}

		else{
			totalByte = bufSize;
		}

		if(totalByte == 0U) return HAL_ERROR;
	}

	status = ICM42688_ReadRegs(handle, ICM42688_UB0_FIFO_DATA, buf, totalByte);
	if(status != HAL_OK) return status;

	*byteRead = totalByte;
	return HAL_OK;
}
