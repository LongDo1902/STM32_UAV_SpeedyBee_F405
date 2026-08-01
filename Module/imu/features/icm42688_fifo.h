/*
 * icm42688_fifo.h
 *
 *  Created on: Mar 13, 2026
 *      Author: dobaolong
 */

#ifndef INC_IMU_ICM42688_FIFO_H_
#define INC_IMU_ICM42688_FIFO_H_

#include "imu/core/icm42688_masks.h"
#include "imu/core/icm42688_registers.h"
#include "imu/core/icm42688_rw.h"
#include "imu/core/icm42688_types.h"
#include "imu/sensors/icm42688_data.h"

/**
 * @brief   Shared FIFO packet-size cache used by FIFO parsing code.
 *          It reflects the most recently decoded packet header and must not be treated as independent parser
 *          state when multiple FIFO streams could be processed concurrently.
 */
extern uint8_t packet_size_;

/**
 * @brief   Configure the byte order used by the FIFO count registers.
 *          The selected ordering is cached in the handle and later controls how FIFO_COUNT1/0 are combined,
 *          so it must match the device setting before any count-dependent read is attempted.
 * @param   pHandle      Pointer to the ICM42688 handle struct.
 * @param   countEndian  Desired FIFO count byte order.
 * @return  true when the register update succeeds, otherwise false.
 */
bool ICM42688_Set_FIFO_Count_Endian(ICM42688_Handle_t *pHandle, ICM42688_FIFO_Count_Endian_t countEndian);

/**
 * @brief   Configure FIFO_COUNT to report bytes or complete records.
 *          The interpretation changes the unit returned by ICM42688_Get_FIFO_Count() and determines which
 *          frame acquisition helper is appropriate; mixing the two modes produces incorrect read lengths.
 * @param   pHandle     Pointer to the ICM42688 handle struct.
 * @param   countRecord Desired FIFO count reporting mode.
 * @return  true when the register update succeeds, otherwise false.
 */
bool ICM42688_Set_FIFO_Count_Rec(ICM42688_Handle_t *pHandle, ICM42688_FIFO_Count_Rec_t countRecord);

/**
 * @brief   Configure the FIFO operating mode.
 *          The selected bypass, stream, or stop-on-full behavior controls how unread data is retained when
 *          storage fills, and the handle cache is updated only after the register write succeeds.
 * @param   pHandle  Pointer to the ICM42688 handle struct.
 * @param   mode     Desired FIFO operating mode.
 * @return  true when the register update succeeds, otherwise false.
 */
bool ICM42688_Set_FIFO_Mode(ICM42688_Handle_t *pHandle, ICM42688_FIFO_Mode_t mode);

/**
 * @brief   Read the current FIFO operating mode.
 *          The FIFO_CONFIG field is decoded directly from hardware and copied into both pMode and the handle
 *          cache so later logic uses the observed device state.
 * @param   pHandle  Pointer to the ICM42688 handle struct.
 * @param   pMode    Pointer to the returned FIFO mode.
 * @return  true when the register read succeeds, otherwise false.
 */
bool ICM42688_Get_FIFO_Mode(ICM42688_Handle_t *pHandle, ICM42688_FIFO_Mode_t *pMode);

/**
 * @brief   Enable or disable gyroscope samples in FIFO packets.
 *          Changing this bit alters which payload fields and packet sizes may appear, so parsing code must
 *          rely on each packet header rather than assuming gyroscope data is always present.
 * @param   pHandle  Pointer to the ICM42688 handle struct.
 * @param   state    Desired gyroscope FIFO state.
 * @return  true when the register update succeeds, otherwise false.
 */
bool ICM42688_Set_FIFO_Gyro_Enable(ICM42688_Handle_t *pHandle, ICM42688_FIFO_GAT_En_t state);

/**
 * @brief   Enable or disable accelerometer samples in FIFO packets.
 *          Changing this bit alters which payload fields and packet sizes may appear, so parsing code must
 *          honor the accelerometer-valid flag decoded from each header.
 * @param   pHandle  Pointer to the ICM42688 handle struct.
 * @param   state    Desired accelerometer FIFO state.
 * @return  true when the register update succeeds, otherwise false.
 */
bool ICM42688_Set_FIFO_Accel_Enable(ICM42688_Handle_t *pHandle, ICM42688_FIFO_GAT_En_t state);

/**
 * @brief   Enable or disable temperature samples in FIFO packets.
 *          FIFO payload selection is independent of powering the temperature sensor, and both settings must
 *          agree when valid temperature samples are required.
 * @param   pHandle  Pointer to the ICM42688 handle struct.
 * @param   state    Desired temperature FIFO state.
 * @return  true when the register update succeeds, otherwise false.
 */
bool ICM42688_Set_FIFO_Temp_Enable(ICM42688_Handle_t *pHandle, ICM42688_FIFO_GAT_En_t state);

/**
 * @brief   Enable or disable high-resolution FIFO packets.
 *          High-resolution mode changes sensor field width and packet length; buffer sizing and header-based
 *          parsing must therefore support the 20-bit packet format before this option is enabled.
 * @param   pHandle  Pointer to the ICM42688 handle struct.
 * @param   state    Desired high-resolution FIFO state.
 * @return  true when the register update succeeds, otherwise false.
 */
bool ICM42688_Set_FIFO_HIRES_Enable(ICM42688_Handle_t *pHandle, ICM42688_FIFO_Hires_En_t state);

/**
 * @brief   Configure one-shot or repeated FIFO watermark interrupt behavior.
 *          Repeat mode can continue asserting while the FIFO remains above threshold, whereas one-shot
 *          behavior requires the condition to clear before another watermark event is generated.
 * @param   pHandle  Pointer to the ICM42688 handle struct.
 * @param   state    Desired FIFO watermark interrupt mode.
 * @return  true when the register update succeeds, otherwise false.
 */
bool ICM42688_Set_FIFO_WM_GT_THS(ICM42688_Handle_t *pHandle, ICM42688_FIFO_WM_Mode_t state);

/**
 * @brief   Configure whether a partial FIFO read resumes from its previous position.
 *          The selected policy changes what happens when a packet is not completely drained, so callers
 *          should not combine partial reads with parsing logic that assumes every read begins at a packet
 *          header.
 * @param   pHandle  Pointer to the ICM42688 handle struct.
 * @param   state    Desired partial-read behavior.
 * @return  true when the register update succeeds, otherwise false.
 */
bool ICM42688_Set_FIFO_Resume_Partial_Read(ICM42688_Handle_t *pHandle, ICM42688_FIFO_Resume_Read_t state);

/**
 * @brief   Set the 12-bit FIFO watermark threshold.
 *          The value is split across FIFO_CONFIG2 and FIFO_CONFIG3 and is rejected when zero or wider than 12
 *          bits. Its unit follows the configured FIFO_COUNT byte/record mode.
 * @param   pHandle        Pointer to the ICM42688 handle struct.
 * @param   fifoWatermark  Desired FIFO watermark in bytes or records, according to FIFO count mode.
 * @return  true when both watermark registers are updated, otherwise false.
 */
bool ICM42688_Set_FIFO_Watermark(ICM42688_Handle_t *pHandle, uint16_t fifoWatermark);

/**
 * @brief   Read the 12-bit FIFO watermark threshold.
 *          The function reconstructs one value from the lower eight and upper four register bits and
 *          refreshes the corresponding handle cache only after both reads succeed.
 * @param   pHandle         Pointer to the ICM42688 handle struct.
 * @param   pFifoWatermark  Pointer to the returned FIFO watermark.
 * @return  true when both watermark registers are read, otherwise false.
 */
bool ICM42688_Get_FIFO_Watermark(ICM42688_Handle_t *pHandle, uint16_t *pFifoWatermark);

/**
 * @brief   Read the current FIFO byte or record count.
 *          The two count bytes are combined according to the configured FIFO count endian setting, and the
 *          returned unit depends on whether byte-count or record-count mode is active.
 * @param   pHandle     Pointer to the ICM42688 handle struct.
 * @param   pFifoCount  Pointer to the returned FIFO count.
 * @return  true when the count registers are read successfully, otherwise false.
 */
bool ICM42688_Get_FIFO_Count(ICM42688_Handle_t *pHandle, uint16_t *pFifoCount);

/**
 * @brief   Decode FIFO packet type and size from a packet header.
 *          Message packets, normal packets, and high-resolution packets are classified from header bits;
 *          invalid or unsupported combinations return false before a payload buffer is accessed.
 * @param   inputHeader  FIFO header byte read from FIFO_DATA.
 * @param   pPacketType  Pointer to the returned packet type.
 * @param   pPacketSize  Pointer to the returned packet size in bytes.
 * @return  true when the header describes a supported packet, otherwise false.
 */
bool ICM42688_Get_FIFO_Packet_Info_From_Header(uint8_t inputHeader, ICM42688_FIFO_Packet_t *pPacketType,
                                               uint8_t *pPacketSize);

/**
 * @brief   Decode one complete FIFO packet into a FIFO frame struct.
 *          Field-valid flags are derived from the header and only present fields are decoded, including sign
 *          extension for 20-bit data. pData must contain at least packetSize valid bytes.
 * @param   pHandle     Pointer to the ICM42688 handle struct.
 * @param   pFrame      Pointer to the decoded FIFO frame output.
 * @param   pData       Pointer to the complete FIFO packet bytes.
 * @param   packetSize  Number of bytes in the packet.
 * @param   packetType  Packet type decoded from the packet header.
 * @return  true when the packet is valid and decoded successfully, otherwise false.
 */
bool ICM42688_FIFO_Parse_Frame(ICM42688_Handle_t *pHandle, ICM42688_FIFO_Frame_t *pFrame,
                               const uint8_t *pData, uint8_t packetSize, ICM42688_FIFO_Packet_t packetType);

/**
 * @brief   Read and decode one FIFO frame when FIFO_COUNT reports records.
 *          The packet header is read first to determine the remaining payload length, then the complete
 *          record is decoded. This helper is not compatible with FIFO_COUNT byte mode.
 * @param   pHandle  Pointer to the ICM42688 handle struct.
 * @param   pFrame   Pointer to the decoded FIFO frame output.
 * @return  true when one complete frame is read and decoded, otherwise false.
 */
bool ICM42688_Get_FIFO_Frame_In_Record(ICM42688_Handle_t *pHandle, ICM42688_FIFO_Frame_t *pFrame);

/**
 * @brief   Read all available FIFO bytes into a caller-provided buffer.
 *          The function obtains FIFO_COUNT and performs one FIFO_DATA burst for exactly that many bytes; it
 *          fails rather than truncating data when the reported count exceeds rawSize.
 * @param   pHandle  Pointer to the ICM42688 handle struct.
 * @param   pRawBuf  Pointer to the raw FIFO output buffer.
 * @param   rawSize  Capacity of pRawBuf in bytes.
 * @return  true when the complete FIFO byte count fits and is read successfully, otherwise false.
 * @note    Valid only when FIFO is not in BYPASS mode, FIFO_COUNT reports bytes, and partial-read resume is
 *          disabled so the complete FIFO contents can be drained in one burst.
 */
bool ICM42688_Get_FIFO_Frame_In_Byte(ICM42688_Handle_t *pHandle, uint8_t *pRawBuf, uint16_t rawSize);

/**
 * @brief   Set or clear the FIFO flush control bit.
 *          Flushing discards unread FIFO contents and resets the count, so request it only when intentionally
 *          starting a clean capture window or recovering from a parsing/overflow condition.
 * @param   pHandle  Pointer to the ICM42688 handle struct.
 * @param   enable   true to request a FIFO flush, false to clear the request.
 * @return  true when the register update succeeds, otherwise false.
 */
bool ICM42688_FIFO_Flush(ICM42688_Handle_t *pHandle, bool enable);

/**
 * @brief   Parse the next FIFO frame from a buffer captured in byte-count mode.
 *          The header at pCurrentPos determines the packet size, bounds are checked against countsInByte, and
 *          the cursor advances only after one complete packet is decoded successfully.
 * @param   pHandle      Pointer to the ICM42688 handle struct.
 * @param   pFrame       Pointer to the decoded FIFO frame output.
 * @param   pByteBuf     Pointer to the raw FIFO byte buffer.
 * @param   countsInByte Number of valid bytes in pByteBuf.
 * @param   pCurrentPos  Pointer to the parsing offset, updated after a successful parse.
 * @return  true when one complete frame is decoded, otherwise false.
 * @note    Populate pByteBuf with ICM42688_Get_FIFO_Frame_In_Byte() and initialize pCurrentPos to zero before
 *          parsing the first frame.
 */
bool ICM42688_FIFO_Parse_One_Byte_Frame(ICM42688_Handle_t *pHandle, ICM42688_FIFO_Frame_t *pFrame,
                                        const uint8_t *pByteBuf, uint16_t countsInByte,
                                        uint16_t *pCurrentPos);

/**
 * @brief   Convert one raw FIFO frame to calibrated physical units.
 *          Only fields marked valid by the parser are converted; accelerometer and gyroscope offsets are
 *          removed before scale factors are applied, while absent outputs remain zero.
 * @param   pHandle             Pointer to the ICM42688 handle struct.
 * @param   pFrame              Pointer to the raw FIFO frame.
 * @param   pOffset             Pointer to the raw accelerometer and gyroscope offsets.
 * @param   pOutCalibratedData  Pointer to the calibrated output sample.
 * @return  true when the valid frame fields are converted successfully, otherwise false.
 */
bool ICM42688_Calibrate_FIFO_Frame(const ICM42688_Handle_t *pHandle, const ICM42688_FIFO_Frame_t *pFrame,
                                   const ICM42688_Offset_Raw_t       *pOffset,
                                   ICM42688_Temp_Accel_Gyro_Scaled_t *pOutCalibratedData);

#endif /* INC_IMU_ICM42688_FIFO_H_ */
