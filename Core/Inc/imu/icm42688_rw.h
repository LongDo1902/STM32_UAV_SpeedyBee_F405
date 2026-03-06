/*
 * icm42688_rw.h
 *
 *  Created on: Mar 5, 2026
 *      Author: dobao
 */

#ifndef INC_IMU_ICM42688_RW_H_
#define INC_IMU_ICM42688_RW_H_

#include "stm32f4xx_hal.h"
#include "icm42688_core.h"

HAL_StatusTypeDef ICM42688_WriteBankAuto(ICM42688_Handle_t* handle, ICM42688_Reg_t encodedReg);

HAL_StatusTypeDef ICM42688_WriteReg(ICM42688_Handle_t* handle, ICM42688_Reg_t encodedReg, uint8_t val);

HAL_StatusTypeDef ICM42688_ReadReg(ICM42688_Handle_t *handle, ICM42688_Reg_t encodedReg, uint8_t* outVal);

HAL_StatusTypeDef ICM42688_ReadRegs(ICM42688_Handle_t* handle, ICM42688_Reg_t startEncodedReg, uint8_t* buf, uint16_t bufLength);

HAL_StatusTypeDef ICM42688_Update_Reg_Bits(ICM42688_Handle_t* handle, ICM42688_Reg_t encodedReg, uint8_t mask, uint8_t value_masked);

#endif /* INC_IMU_ICM42688_RW_H_ */
