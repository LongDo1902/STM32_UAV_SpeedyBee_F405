/*
 * icm42688_rw.h
 *
 *  Created on: Mar 5, 2026
 *      Author: dobao
 */

#ifndef INC_IMU_ICM42688_RW_H_
#define INC_IMU_ICM42688_RW_H_

#include "imu/core/icm42688_defs.h"
#include "imu/core/icm42688_registers.h"
#include "imu/core/icm42688_types.h"

void ICM42688_CS_Pull_Low(GPIO_TypeDef *csPort, uint16_t csPin);

void ICM42688_CS_Pull_High(GPIO_TypeDef *csPort, uint16_t csPin);

bool ICM42688_WriteReg(ICM42688_Handle_t *handle, ICM42688_Reg_t encodedReg, uint8_t val);

bool ICM42688_ReadReg(ICM42688_Handle_t *handle, ICM42688_Reg_t encodedReg, uint8_t *outVal);

bool ICM42688_ReadRegs(ICM42688_Handle_t *handle, ICM42688_Reg_t startEncodedReg, uint8_t *buf, uint16_t bufLength);

bool ICM42688_Update_Reg_Bits(ICM42688_Handle_t *handle, ICM42688_Reg_t encodedReg, uint8_t mask, uint8_t valueMasked);

bool ICM42688_ReadRegs_DMA_Start(bool autoBankSelect);

bool ICM42688_WriteReg_DMA_Start();

bool ICM42688_WriteRegs_DMA_Start();

void ICM42688_DMA_End();

#endif /* INC_IMU_ICM42688_RW_H_ */
