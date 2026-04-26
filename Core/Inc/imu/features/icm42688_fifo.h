/*
 * icm42688_fifo.h
 *
 *  Created on: Mar 13, 2026
 *      Author: dobao
 */

#ifndef INC_IMU_ICM42688_FIFO_H_
#define INC_IMU_ICM42688_FIFO_H_

#include "imu/core/icm42688_masks.h"
#include "imu/core/icm42688_registers.h"
#include "imu/core/icm42688_rw.h"
#include "imu/core/icm42688_types.h"

ICM42688_Status_t
ICM42688_Set_FIFO_Count_Endian(ICM42688_Handle_t *handle, ICM42688_FIFO_Count_Endian_t countEndian);

ICM42688_Status_t
ICM42688_Set_FIFO_Count_Rec(ICM42688_Handle_t *handle, ICM42688_FIFO_Count_Rec_t countRecord);

ICM42688_Status_t
ICM42688_Set_FIFO_Mode(ICM42688_Handle_t *handle, ICM42688_FIFO_Mode_t mode);

ICM42688_Status_t
ICM42688_Get_FIFO_Mode(ICM42688_Handle_t *handle, ICM42688_FIFO_Mode_t *mode);

ICM42688_Status_t
ICM42688_Set_FIFO_Gyro_Enable(ICM42688_Handle_t *handle, ICM42688_FIFO_GAT_En_t state);

ICM42688_Status_t
ICM42688_Set_FIFO_Accel_Enable(ICM42688_Handle_t *handle, ICM42688_FIFO_GAT_En_t state);

ICM42688_Status_t
ICM42688_Set_FIFO_Temp_Enable(ICM42688_Handle_t *handle, ICM42688_FIFO_GAT_En_t state);

ICM42688_Status_t
ICM42688_Set_FIFO_HIRES_Enable(ICM42688_Handle_t *handle, ICM42688_FIFO_Hires_En_t state);

ICM42688_Status_t
ICM42688_Set_FIFO_WM_GT_THS(ICM42688_Handle_t *handle, ICM42688_FIFO_WM_Mode_t state);

ICM42688_Status_t
ICM42688_Set_FIFO_Resume_Partial_Read(ICM42688_Handle_t *handle, ICM42688_FIFO_Resume_Read_t state);

ICM42688_Status_t
ICM42688_Set_FIFO_Watermark(ICM42688_Handle_t *handle, uint16_t fifoWatermark);

ICM42688_Status_t
ICM42688_Get_FIFO_Watermark(ICM42688_Handle_t *handle, uint16_t *fifoWatermark);

ICM42688_Status_t
ICM42688_Get_FIFO_Count(ICM42688_Handle_t *handle, uint16_t *fifoCount);

ICM42688_Status_t
ICM42688_Get_FIFO_Packet_Info_From_Header(uint8_t inputHeader, ICM42688_FIFO_Packet_t *packetType,
                                          uint8_t *packetSize);

ICM42688_Status_t
ICM42688_FIFO_Parse_Frame(ICM42688_Handle_t *handle, ICM42688_FIFO_Frame_t *frame,
                          const uint8_t *data, uint8_t packetSize,
                          ICM42688_FIFO_Packet_t packetType);

ICM42688_Status_t
ICM42688_Get_FIFO_Frame_In_Record(ICM42688_Handle_t *handle, ICM42688_FIFO_Frame_t *frame);

ICM42688_Status_t
ICM42688_Get_FIFO_Frame_In_Byte(ICM42688_Handle_t *handle, uint8_t *rawFIFOBuf, uint16_t rawBufSize);

ICM42688_Status_t
ICM42688_FIFO_Parse_One_Byte_Frame(ICM42688_Handle_t *handle, ICM42688_FIFO_Frame_t *FIFO_frame,
                                   const uint8_t *FIFO_byteBuf, uint16_t FIFOCountInByte,
                                   uint16_t *currentPos);

#endif /* INC_IMU_ICM42688_FIFO_H_ */
