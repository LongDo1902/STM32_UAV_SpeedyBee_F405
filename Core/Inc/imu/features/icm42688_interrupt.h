/*
 * icm42688_interrupt.h
 *
 *  Created on: Mar 13, 2026
 *      Author: dobao
 */

#ifndef INC_IMU_ICM42688_INTERRUPT_H_
#define INC_IMU_ICM42688_INTERRUPT_H_

#include "imu/core/icm42688_masks.h"
#include "imu/core/icm42688_registers.h"
#include "imu/core/icm42688_rw.h"
#include "imu/core/icm42688_types.h"

HAL_StatusTypeDef
ICM42688_Set_Int1_Config(ICM42688_Handle_t *handle, ICM42688_Int_Polarity_t polarity,
                         ICM42688_Int_Drive_Circuit_t drive, ICM42688_Int_Mode_t mode);

HAL_StatusTypeDef
ICM42688_Set_Int2_Config(ICM42688_Handle_t *handle, ICM42688_Int_Polarity_t polarity,
                         ICM42688_Int_Drive_Circuit_t drive, ICM42688_Int_Mode_t mode);

HAL_StatusTypeDef
ICM42688_Get_Int_Status(ICM42688_Handle_t *handle, uint8_t *outStatus);

bool
ICM42688_Int_Status_Has(uint8_t status, ICM42688_Int_Status_t intState);

HAL_StatusTypeDef
ICM42688_Set_Int1_FIFO_Full_Enable(ICM42688_Handle_t *handle, bool enable);

HAL_StatusTypeDef
ICM42688_Set_Int1_FIFO_Threshold_Enable(ICM42688_Handle_t *handle, bool enable);

HAL_StatusTypeDef
ICM42688_Set_Int1_DataReady_Enable(ICM42688_Handle_t *handle, bool enable);

HAL_StatusTypeDef
ICM42688_Set_Int1_ResetDone_Enable(ICM42688_Handle_t *handle, bool enable);

#endif /* INC_IMU_ICM42688_INTERRUPT_H_ */
