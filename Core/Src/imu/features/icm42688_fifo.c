/*
 * icm42688_fifo.c
 *
 *  Created on: Mar 12, 2026
 *      Author: dobao
 */

#include "imu/features/icm42688_fifo.h"
#include "string.h"

/* ==================================================================================
 *	FIFO CONFIG
 * ================================================================================== */

/**
 * @brief   Set a desired FIFO count endian type
 * @param   handle          Pointer to ICM42688 Handle struct
 * @param   countEndian     Desired count endian (BIG/LITTLE) to be written
 */
ICM42688_Status_t
ICM42688_Set_FIFO_Count_Endian(ICM42688_Handle_t *handle, ICM42688_FIFO_Count_Endian_t countEndian)
{
    if (!handle)
        return ICM42688_ERROR;

    if (((uint8_t)countEndian != 0U && (uint8_t)countEndian != 1U))
        return ICM42688_ERROR;

    HAL_StatusTypeDef status =
        ICM42688_Update_Reg_Bits(handle, ICM42688_UB0_INTF_CONF0, ICM42688_FIFO_COUNT_ENDIAN_Msk,
                                 ICM42688_FIFO_COUNT_ENDIAN_Val(countEndian));
    if (status != HAL_OK)
        return ICM42688_ERROR;

    handle->fifo_config.fifo_count_endian = countEndian;

    return ICM42688_OK;
}



/**
 * @brief   Set a desired count record type
 * @param   handle      Pointer to ICM42688 Handle struct
 * @param   countRecord Number of receiving data is considered count in byte/record
 */
ICM42688_Status_t
ICM42688_Set_FIFO_Count_Rec(ICM42688_Handle_t *handle, ICM42688_FIFO_Count_Rec_t countRecord)
{
    if (!handle)
        return ICM42688_ERROR;

    HAL_StatusTypeDef status =
        ICM42688_Update_Reg_Bits(handle, ICM42688_UB0_INTF_CONF0, ICM42688_FIFO_COUNT_REC_Msk,
                                 ICM42688_FIFO_COUNT_REC_Val(countRecord));
    if (status != HAL_OK)
        return ICM42688_ERROR;

    handle->fifo_config.fifo_count_rec = countRecord;

    return ICM42688_OK;
}



/**
 * @brief   Set a desired FIFO mode
 * @param   handle  Pointer to ICM42688 Handle struct
 * @param   mode    Desired FIFO working mode
 */
ICM42688_Status_t
ICM42688_Set_FIFO_Mode(ICM42688_Handle_t *handle, ICM42688_FIFO_Mode_t mode)
{
    if (!handle)
        return ICM42688_ERROR;

    if ((uint8_t)mode > (uint8_t)STOP_ON_FULL)
        return ICM42688_ERROR;

    HAL_StatusTypeDef status = ICM42688_Update_Reg_Bits(
        handle, ICM42688_UB0_FIFO_CONF, ICM42688_FIFO_MODE_Msk, ICM42688_FIFO_MODE_Val(mode));

    if (status != HAL_OK)
        return ICM42688_ERROR;

    handle->fifo_config.fifo_mode = (ICM42688_FIFO_Mode_t)mode;

    return ICM42688_OK;
}



/**
 * @brief   Read the current FIFO working mode
 * @param   handle  Pointer to ICM42688 Handle struct
 * @param   mode    Pointer to a variable that stores the actual FIFO working mode
 */
ICM42688_Status_t
ICM42688_Get_FIFO_Mode(ICM42688_Handle_t *handle, ICM42688_FIFO_Mode_t *mode)
{
    if (!handle || !mode)
        return ICM42688_ERROR;

    uint8_t           reg    = 0U;
    HAL_StatusTypeDef status = ICM42688_ReadReg(handle, ICM42688_UB0_FIFO_CONF, &reg);
    if (status != HAL_OK)
        return ICM42688_ERROR;

    uint8_t raw_mode = (uint8_t)((reg & ICM42688_FIFO_MODE_Msk) >> ICM42688_FIFO_MODE_Pos);
    bool    is_mode_stop_on_full = (raw_mode == 2U) || (raw_mode == 3U);

    *mode = (is_mode_stop_on_full) ? STOP_ON_FULL : (ICM42688_FIFO_Mode_t)raw_mode;

    handle->fifo_config.fifo_mode = (ICM42688_FIFO_Mode_t)*mode;

    return ICM42688_OK;
}



/**
 * @brief   Enable FIFO for Gyro sensor
 * @param   handle  Pointer to ICM42688 Handle struct
 * @param   state   Enable or Disable
 */
ICM42688_Status_t
ICM42688_Set_FIFO_Gyro_Enable(ICM42688_Handle_t *handle, ICM42688_FIFO_GAT_En_t state)
{
    if (!handle)
        return ICM42688_ERROR;

    HAL_StatusTypeDef status =
        ICM42688_Update_Reg_Bits(handle, ICM42688_UB0_FIFO_CONF1, ICM42688_FIFO_GYRO_EN_Msk,
                                 ICM42688_FIFO_GYRO_EN_Val(state));
    if (status != HAL_OK)
        return ICM42688_ERROR;

    handle->fifo_config.fifo_gyro_state = (ICM42688_FIFO_GAT_En_t)state;

    return ICM42688_OK;
}



/**
 * @brief   Enable FIFO for Accel sensor
 * @param   handle      Pointer to ICM42688 Handle struct
 * @param   state       Enable or Disable
 */
ICM42688_Status_t
ICM42688_Set_FIFO_Accel_Enable(ICM42688_Handle_t *handle, ICM42688_FIFO_GAT_En_t state)
{
    if (!handle)
        return ICM42688_ERROR;

    HAL_StatusTypeDef status =
        ICM42688_Update_Reg_Bits(handle, ICM42688_UB0_FIFO_CONF1, ICM42688_FIFO_ACCEL_EN_Msk,
                                 ICM42688_FIFO_ACCEL_EN_Val(state));
    if (status != HAL_OK)
        return ICM42688_ERROR;

    handle->fifo_config.fifo_accel_state = (ICM42688_FIFO_GAT_En_t)state;

    return ICM42688_OK;
}



/**
 * @brief   Enable FIFO for Temperature sensor
 * @param   handle      Pointer to ICM42688 Handle struct
 * @param   state       Enable or Disable
 */
ICM42688_Status_t
ICM42688_Set_FIFO_Temp_Enable(ICM42688_Handle_t *handle, ICM42688_FIFO_GAT_En_t state)
{
    if (!handle)
        return ICM42688_ERROR;

    HAL_StatusTypeDef status =
        ICM42688_Update_Reg_Bits(handle, ICM42688_UB0_FIFO_CONF1, ICM42688_FIFO_TEMP_EN_Msk,
                                 ICM42688_FIFO_TEMP_EN_Val(state));
    if (status != HAL_OK)
        return ICM42688_ERROR;

    handle->fifo_config.fifo_temp_state = (ICM42688_FIFO_GAT_En_t)state;

    return ICM42688_OK;
}



/**
 * @brief   Enable high-resolution data / more bit data for Gyro, Accel and Temperature
 * @param   handle      Pointer to ICM42688 Handle struct
 * @param   state       Enable or Disable
 */
ICM42688_Status_t
ICM42688_Set_FIFO_HIRES_Enable(ICM42688_Handle_t *handle, ICM42688_FIFO_Hires_En_t state)
{
    if (!handle)
        return ICM42688_ERROR;

    HAL_StatusTypeDef status =
        ICM42688_Update_Reg_Bits(handle, ICM42688_UB0_FIFO_CONF1, ICM42688_FIFO_HIRES_EN_Msk,
                                 ICM42688_FIFO_HIRES_EN_Val(state));
    if (status != HAL_OK)
        return ICM42688_ERROR;

    handle->fifo_config.fifo_hires_state = (ICM42688_FIFO_Hires_En_t)state;

    return ICM42688_OK;
}



/**
 * @brief   Set "FIFO Watermark Greater than Threshold" oneshot or repeat interrupt mode
 * @param   handle  Pointer to ICM42688 Handle struct
 * @param   state   Oneshot or repeat interrupt
 */
ICM42688_Status_t
ICM42688_Set_FIFO_WM_GT_THS(ICM42688_Handle_t *handle, ICM42688_FIFO_WM_Mode_t state)
{
    if (!handle)
        return ICM42688_ERROR;

    HAL_StatusTypeDef status =
        ICM42688_Update_Reg_Bits(handle, ICM42688_UB0_FIFO_CONF1, ICM42688_FIFO_WM_GT_TH_Msk,
                                 ICM42688_FIFO_WM_GT_TH_Val(state));
    if (status != HAL_OK)
        return ICM42688_ERROR;

    handle->fifo_config.fifo_wm_mode = (ICM42688_FIFO_WM_Mode_t)state;

    return ICM42688_OK;
}



/**
 * @brief   Set "FIFO Resume Partial Read" mode
 * @param   handle  Pointer to ICM42688 Handle struct
 * @param   state   Whole register re-read or partial register read resume
 */
ICM42688_Status_t
ICM42688_Set_FIFO_Resume_Partial_Read(ICM42688_Handle_t *handle, ICM42688_FIFO_Resume_Read_t state)
{
    if (!handle)
        return ICM42688_ERROR;
    HAL_StatusTypeDef status = ICM42688_Update_Reg_Bits(handle, ICM42688_UB0_FIFO_CONF1,
                                                        ICM42688_FIFO_RESUME_PARTIAL_RD_Msk,
                                                        ICM42688_FIFO_RESUME_PARTIAL_RD_Val(state));
    if (status != HAL_OK)
        return ICM42688_ERROR;

    handle->fifo_config.fifo_partial_read_state = (ICM42688_FIFO_Resume_Read_t)state;

    return ICM42688_OK;
}



/**
 * @brief   Set a desired watermark for FIFO
 * @param   handle          Pointer to ICM42688 Handle struct
 * @param   fifoWatermark   Desired watermark level
 */
ICM42688_Status_t
ICM42688_Set_FIFO_Watermark(ICM42688_Handle_t *handle, uint16_t fifoWatermark)
{
    if (!handle)
        return ICM42688_ERROR;

    if ((fifoWatermark == 0U) || (fifoWatermark > 0x0FFFU))
        return ICM42688_ERROR;

    uint8_t fifo_lower_wm = (uint8_t)(fifoWatermark & 0x00FFU);
    uint8_t fifo_upper_wm = (uint8_t)((fifoWatermark >> 8) & 0x0FU);

    HAL_StatusTypeDef status =
        ICM42688_Update_Reg_Bits(handle, ICM42688_UB0_FIFO_CONF2, ICM42688_FIFO_WM_LOWER_Msk,
                                 ICM42688_FIFO_WM_LOWER_Val(fifo_lower_wm));
    if (status != HAL_OK)
        return ICM42688_ERROR;

    status = ICM42688_Update_Reg_Bits(handle, ICM42688_UB0_FIFO_CONF3, ICM42688_FIFO_WM_UPPER_Msk,
                                      ICM42688_FIFO_WM_UPPER_Val(fifo_upper_wm));
    if (status != HAL_OK)
        return ICM42688_ERROR;

    handle->fifo_config.fifo_watermark = (uint16_t)fifoWatermark;

    return ICM42688_OK;
}



/**
 * @brief   Read/Get the current watermark level
 * @param   handle          Pointer to ICM42688 Handle struct
 * @param   fifoWatermark   Pointer to a variable that stores the actual returned WM level
 */
ICM42688_Status_t
ICM42688_Get_FIFO_Watermark(ICM42688_Handle_t *handle, uint16_t *fifoWatermark)
{
    if (!handle || !fifoWatermark)
        return ICM42688_ERROR;

    uint8_t           fifo_wm_buf[2];
    HAL_StatusTypeDef status = ICM42688_ReadRegs(handle, ICM42688_UB0_FIFO_CONF2, fifo_wm_buf, 2);
    if (status != HAL_OK)
        return ICM42688_ERROR;

    *fifoWatermark = (uint16_t)((uint16_t)((fifo_wm_buf[1] & ICM42688_FIFO_WM_UPPER_Msk) << 8U) |
                                (uint16_t)((fifo_wm_buf[0] & ICM42688_FIFO_WM_LOWER_Msk)));

    handle->fifo_config.fifo_watermark = (uint16_t)*fifoWatermark;

    return ICM42688_OK;
}



/**
 * @brief   Read the actual return number of bytes/records from the sensor
 * @param   handle      Pointer to ICM42688 Handle struct
 * @param   fifoCount   Pointer to a variable that stores the actual return number of bytes/records
 */
ICM42688_Status_t
ICM42688_Get_FIFO_Count(ICM42688_Handle_t *handle, uint16_t *fifoCount)
{
    if (!handle || !fifoCount)
        return ICM42688_ERROR;

    uint8_t           fifo_count_buf[2];
    HAL_StatusTypeDef status =
        ICM42688_ReadRegs(handle, ICM42688_UB0_FIFO_COUNTH, fifo_count_buf, 2);
    if (status != HAL_OK)
        return ICM42688_ERROR;

    if (handle->fifo_config.fifo_count_endian == FIFO_COUNT_BIG_ENDIAN) { // Big endian format
        *fifoCount = (uint16_t)(((uint16_t)(fifo_count_buf[0] & 0xFFU) << 8) |
                                ((uint16_t)(fifo_count_buf[1] & 0xFFU)));
    }
    else {
        *fifoCount = (uint16_t)(((uint16_t)(fifo_count_buf[1] & 0xFFU) << 8) |
                                ((uint16_t)(fifo_count_buf[0] & 0xFFU)));
    }

    handle->fifo_config.fifo_count = (uint16_t)*fifoCount;

    return ICM42688_OK;
}



static inline bool
ICM42688_FIFO_Header_Has(uint8_t header, uint8_t mask)
{
    return ((header & mask) != 0U);
}



static inline uint8_t
ICM42688_Get_FIFO_TimestampFsync_Mode(uint8_t header)
{
    return (uint8_t)((header & ICM42688_FIFO_HEADER_TIMESTAMP_FSYNC_Msk) >>
                     ICM42688_FIFO_HEADER_TIMESTAMP_FSYNC_Pos);
}



static inline uint16_t
ICM42688_Decode_BE16_Unsigned(const uint8_t msb, const uint8_t lsb)
{
    return (uint16_t)((msb << 8) | (lsb));
}



static inline int16_t
ICM42688_Decode_BE16_Signed(const uint8_t msb, const uint8_t lsb)
{
    return (int16_t)((msb << 8) | (lsb));
}



static inline int32_t
ICM42688_SignExtend20(uint32_t value)
{
    if ((value & 0x80000UL) != 0U) {
        value |= 0xFFF00000UL;
    }
    return (int32_t)value;
}



/**
 * @brief   Extract information from FIFO header including @p packetType and @p packetSize
 * @param   inputHeader     An input variable that carries the actual 8 bits value
 * @param   packetType      Pointer to an output variable that stores the FIFO packet type from
 *                          decoding FIFO header
 * @param   packetSize      Pointer to an output variable that stores the FIFO packet size based on
 *                          FIFO packet type
 */
ICM42688_Status_t
ICM42688_Get_FIFO_Packet_Info_From_Header(uint8_t inputHeader, ICM42688_FIFO_Packet_t *packetType,
                                          uint8_t *packetSize)
{
    if (!packetType || !packetSize)
        return ICM42688_ERROR;

    const bool has_msg   = ICM42688_FIFO_Header_Has(inputHeader, ICM42688_FIFO_HEADER_MSG_Msk);
    const bool has_accel = ICM42688_FIFO_Header_Has(inputHeader, ICM42688_FIFO_HEADER_ACCEL_Msk);
    const bool has_gyro  = ICM42688_FIFO_Header_Has(inputHeader, ICM42688_FIFO_HEADER_GYRO_Msk);
    const bool has_20bit = ICM42688_FIFO_Header_Has(inputHeader, ICM42688_FIFO_HEADER_20_Msk);

    if (has_msg) {
        *packetType = FIFO_PACKET_INVALID;
        *packetSize = 0U;
        return ICM42688_ERROR;
    }

    // At lease HEADER_ACCEL and HEADER GYRO must be set to be valid
    if (!has_accel && !has_gyro) {
        *packetType = FIFO_PACKET_INVALID;
        *packetSize = 0U;
        return ICM42688_ERROR;
    }

    // Packet 1: Accel only, 8 bytes
    if (has_accel && !has_gyro && !has_20bit) {
        *packetType = FIFO_PACKET_1;
        *packetSize = 8U;
        return ICM42688_OK;
    }

    // Packet 2: Gyro only, 8 bytes
    if (has_gyro && !has_accel && !has_20bit) {
        *packetType = FIFO_PACKET_2;
        *packetSize = 8U;
        return ICM42688_OK;
    }

    // Packet 3: Accel + Gyro 16 bytes
    if (has_accel && has_gyro && !has_20bit) {
        *packetType = FIFO_PACKET_3;
        *packetSize = 16U;
        return ICM42688_OK;
    }

    // Packet 4: Accel + Gyro + 20 bytes
    if (has_accel && has_gyro && has_20bit) {
        *packetType = FIFO_PACKET_4;
        *packetSize = 20U;
        return ICM42688_OK;
    }

    // No valid info from FIFO header
    *packetType = FIFO_PACKET_INVALID;
    *packetSize = 0U;

    return ICM42688_ERROR;
}



/**
 * @brief   Parsing inputs and fill thems into corresponding fields in @p frame
 * @param   handle      Pointer to ICM42688 Handle struct
 * @param   frame       Pointer to FIFO frame that contains all properties/data of a FIFO
 * @param   data        Pointer to input FIFO buffer which is consecutively read from FIFO_DATA
 *                      register
 * @param   packetType  Pointer to an input variable that stores FIFO packet type
 * @param   packetSize  Pointer to an input variable that stores FIFO packet size
 */
// Maybe this name ICM42688_FIFO_Decode_Frame is better
ICM42688_Status_t
ICM42688_FIFO_Parse_Frame(ICM42688_Handle_t *handle, ICM42688_FIFO_Frame_t *frame,
                          const uint8_t *data, uint8_t packetSize,
                          ICM42688_FIFO_Packet_t packetType)
{
    if (!handle || !data || !frame)
        return ICM42688_ERROR;

    // Clean the leftover members in frame to start a known/clean state before the real parsing
    memset(frame, 0, sizeof(*frame));

    // Fill information into FIFO frame
    frame->header      = data[0];
    frame->packet_type = packetType;
    frame->packet_size = packetSize;

    frame->timestamp_fsync_mode = ICM42688_Get_FIFO_TimestampFsync_Mode(frame->header);

    frame->odr_accel_changed =
        ICM42688_FIFO_Header_Has(frame->header, ICM42688_FIFO_HEADER_ODR_ACCEL_Msk);

    frame->odr_gyro_changed =
        ICM42688_FIFO_Header_Has(frame->header, ICM42688_FIFO_HEADER_ODR_GYRO_Msk);

    // Copy every valid byte into another buffer in frame
    for (uint16_t i = 0U; i < packetSize; i++) {
        frame->raw[i] = data[i];
    }

    /* timestamp/fsync mode:
     * 		00 none
     * 		01 reserved
     * 		10 ODR timestamp
     * 		11 FSYNC time */
    if (frame->timestamp_fsync_mode == 1U) {
        return ICM42688_ERROR;
    }

    frame->timestamp_valid =
        (frame->timestamp_fsync_mode == 2U) || (frame->timestamp_fsync_mode == 3U);

    frame->timestamp_fsync_valid = (frame->timestamp_fsync_mode == 3U);

    switch (frame->packet_type) {
        case FIFO_PACKET_1: {
            frame->accel_valid = true;
            frame->gyro_valid  = false;
            frame->temp_valid  = true;
            frame->hires_valid = false;

            frame->accel_raw16[0] = ICM42688_Decode_BE16_Signed(data[1], data[2]);
            frame->accel_raw16[1] = ICM42688_Decode_BE16_Signed(data[3], data[4]);
            frame->accel_raw16[2] = ICM42688_Decode_BE16_Signed(data[5], data[6]);

            // Convert raw accel to accel in g
            if (handle->accel_g_per_lsb > 0.0f) {
                float s = handle->accel_g_per_lsb;

                frame->gat_scaled.accel_g[0] = (float)(frame->accel_raw16[0] * s);
                frame->gat_scaled.accel_g[1] = (float)(frame->accel_raw16[1] * s);
                frame->gat_scaled.accel_g[2] = (float)(frame->accel_raw16[2] * s);
            }

            frame->temp_raw8         = (int8_t)data[7];
            frame->gat_scaled.temp_c = ((float)frame->temp_raw8 / 2.07f) + 25.0f;

            break;
        }

        case FIFO_PACKET_2: {
            frame->accel_valid = false;
            frame->gyro_valid  = true;
            frame->temp_valid  = true;
            frame->hires_valid = false;

            frame->gyro_raw16[0] = ICM42688_Decode_BE16_Signed(data[1], data[2]);
            frame->gyro_raw16[1] = ICM42688_Decode_BE16_Signed(data[3], data[4]);
            frame->gyro_raw16[2] = ICM42688_Decode_BE16_Signed(data[5], data[6]);

            // Convert raw gyro to gyro in dps
            if (handle->gyro_dps_per_lsb > 0.0f) {
                float s = handle->gyro_dps_per_lsb;

                frame->gat_scaled.gyro_dps[0] = (float)(frame->gyro_raw16[0] * s);
                frame->gat_scaled.gyro_dps[1] = (float)(frame->gyro_raw16[1] * s);
                frame->gat_scaled.gyro_dps[2] = (float)(frame->gyro_raw16[2] * s);
            }

            frame->temp_raw8         = (int8_t)data[7];
            frame->gat_scaled.temp_c = ((float)frame->temp_raw8 / 2.07f) + 25.0f;

            break;
        }

        case FIFO_PACKET_3: {
            frame->accel_valid = true;
            frame->gyro_valid  = true;
            frame->temp_valid  = true;
            frame->hires_valid = false;

            frame->accel_raw16[0] = ICM42688_Decode_BE16_Signed(data[1], data[2]);
            frame->accel_raw16[1] = ICM42688_Decode_BE16_Signed(data[3], data[4]);
            frame->accel_raw16[2] = ICM42688_Decode_BE16_Signed(data[5], data[6]);

            frame->gyro_raw16[0] = ICM42688_Decode_BE16_Signed(data[7], data[8]);
            frame->gyro_raw16[1] = ICM42688_Decode_BE16_Signed(data[9], data[10]);
            frame->gyro_raw16[2] = ICM42688_Decode_BE16_Signed(data[11], data[12]);

            // Convert raw accel and gyro to accel(g) and gyro(dps)
            if ((handle->gyro_dps_per_lsb > 0.0f) && (handle->accel_g_per_lsb > 0.0f)) {
                float sa = handle->accel_g_per_lsb;
                float sg = handle->gyro_dps_per_lsb;

                frame->gat_scaled.accel_g[0] = (float)(frame->accel_raw16[0] * sa);
                frame->gat_scaled.accel_g[1] = (float)(frame->accel_raw16[1] * sa);
                frame->gat_scaled.accel_g[2] = (float)(frame->accel_raw16[2] * sa);

                frame->gat_scaled.gyro_dps[0] = (float)(frame->gyro_raw16[0] * sg);
                frame->gat_scaled.gyro_dps[1] = (float)(frame->gyro_raw16[1] * sg);
                frame->gat_scaled.gyro_dps[2] = (float)(frame->gyro_raw16[2] * sg);
            }

            frame->temp_raw8         = (int8_t)data[13];
            frame->gat_scaled.temp_c = ((float)frame->temp_raw8 / 2.07f) + 25.0f;

            frame->timestamp = ICM42688_Decode_BE16_Unsigned(data[14], data[15]);

            break;
        }

        case FIFO_PACKET_4: {
            frame->accel_valid = true;
            frame->gyro_valid  = true;
            frame->temp_valid  = true;
            frame->hires_valid = true;

            uint32_t accel_x_raw = (uint32_t)(data[1] << 12) | (uint32_t)(data[2] << 4) |
                                   (uint32_t)((data[17] >> 4) & 0x0F);

            uint32_t accel_y_raw = (uint32_t)(data[3] << 12) | (uint32_t)(data[4] << 4) |
                                   (uint32_t)((data[18] >> 4) & 0x0F);

            uint32_t accel_z_raw = (uint32_t)(data[5] << 12) | (uint32_t)(data[6] << 4) |
                                   (uint32_t)((data[19] >> 4) & 0x0F);

            uint32_t gyro_x_raw =
                (uint32_t)(data[7] << 12) | (uint32_t)(data[8] << 4) | (uint32_t)(data[17] & 0x0F);

            uint32_t gyro_y_raw =
                (uint32_t)(data[9] << 12) | (uint32_t)(data[10] << 4) | (uint32_t)(data[18] & 0x0F);

            uint32_t gyro_z_raw = (uint32_t)(data[11] << 12) | (uint32_t)(data[12] << 4) |
                                  (uint32_t)(data[19] & 0x0F);

            frame->accel_raw20[0] = ICM42688_SignExtend20(accel_x_raw);
            frame->accel_raw20[1] = ICM42688_SignExtend20(accel_y_raw);
            frame->accel_raw20[2] = ICM42688_SignExtend20(accel_z_raw);

            frame->gyro_raw20[0] = ICM42688_SignExtend20(gyro_x_raw);
            frame->gyro_raw20[1] = ICM42688_SignExtend20(gyro_y_raw);
            frame->gyro_raw20[2] = ICM42688_SignExtend20(gyro_z_raw);

            // Convert raw accel and gyro to accel(g) and gyro(dps)
            const float accel_g_per_lsb_p4  = 1.0f / 8192.0f;
            const float gyro_dps_per_lsb_p4 = 1.0f / 131.0f;

            frame->gat_scaled.accel_g[0] = accel_g_per_lsb_p4 * (frame->accel_raw20[0]);
            frame->gat_scaled.accel_g[1] = accel_g_per_lsb_p4 * (frame->accel_raw20[1]);
            frame->gat_scaled.accel_g[2] = accel_g_per_lsb_p4 * (frame->accel_raw20[2]);

            frame->gat_scaled.gyro_dps[0] = gyro_dps_per_lsb_p4 * (frame->gyro_raw20[0]);
            frame->gat_scaled.gyro_dps[1] = gyro_dps_per_lsb_p4 * (frame->gyro_raw20[1]);
            frame->gat_scaled.gyro_dps[2] = gyro_dps_per_lsb_p4 * (frame->gyro_raw20[2]);

            frame->temp_raw16        = ICM42688_Decode_BE16_Signed(data[13], data[14]);
            frame->gat_scaled.temp_c = (float)((frame->temp_raw16) / 132.48f) + 25.0f;

            frame->timestamp = ICM42688_Decode_BE16_Unsigned(data[15], data[16]);

            break;
        }

        default:
            return ICM42688_ERROR;
    }

    return ICM42688_OK;
}



/**
 * @brief
 * @param   handle  Pointer to ICM42688 Handle struct
 * @param   frame   Pointer to an output ICM42688 FIFO frame struct
 */
ICM42688_Status_t
ICM42688_Get_FIFO_Frame_In_Record(ICM42688_Handle_t *handle, ICM42688_FIFO_Frame_t *frame)
{
    if (!handle || !frame)
        return ICM42688_ERROR;

    // FIFO mode must not be BYPASS
    if (handle->fifo_config.fifo_mode == BYPASS)
        return ICM42688_ERROR;

    // Must be count in record
    if (handle->fifo_config.fifo_count_rec != FIFO_COUNT_IN_RECORD)
        return ICM42688_ERROR;

    // Must enable resume partial FIFO read
    if (handle->fifo_config.fifo_partial_read_state != FIFO_PARTIAL_READ_ENABLE)
        return ICM42688_ERROR;

    // Get total available number of packets/records
    uint16_t fifo_count_in_record = 0U;

    HAL_StatusTypeDef status = ICM42688_Get_FIFO_Count(handle, &fifo_count_in_record);
    if (status != HAL_OK)
        return ICM42688_ERROR;

    if (fifo_count_in_record < 1U)
        return ICM42688_ERROR;

    // Extract Packet Type & Packet Size from header by reading the first byte from register
    uint8_t                header           = 0U;
    ICM42688_FIFO_Packet_t fifo_packet_type = FIFO_PACKET_INVALID;
    uint8_t                fifo_packet_size = 0U;

    // Append the first 8-bits field from FIFO_DATA to @p header
    status = ICM42688_ReadRegs(handle, ICM42688_UB0_FIFO_DATA, &header, 1);
    if (status != HAL_OK)
        return ICM42688_ERROR;

    status =
        ICM42688_Get_FIFO_Packet_Info_From_Header(header, &fifo_packet_type, &fifo_packet_size);
    if (status != HAL_OK)
        return ICM42688_ERROR;

    if ((fifo_packet_size == 0U) || (fifo_packet_type == FIFO_PACKET_INVALID))
        return ICM42688_ERROR;

    // Resume reading the remaining bytes in that packet
    uint8_t fifo_data[20] = {0};
    fifo_data[0]          = header;
    status                = ICM42688_ReadRegs(handle, ICM42688_UB0_FIFO_DATA, &fifo_data[1],
                                              (uint16_t)(fifo_packet_size - 1U));
    if (status != HAL_OK)
        return ICM42688_ERROR;

    // Start parsing frame
    status =
        ICM42688_FIFO_Parse_Frame(handle, frame, fifo_data, fifo_packet_size, fifo_packet_type);
    if (status != HAL_OK)
        return ICM42688_ERROR;

    return ICM42688_OK;
}



/**
 * @brief   Read all availale FIFO bytes in one SPI burst
 * @note    This function is valid only:
 *          FIFO is enabled, NOT BYPASS
 *          FIFO count is configured to report in BYTEs
 *          PARTIAL FIFO READ is DISABLED
 * In this mode, FIFO_COUNT gives the total number of bytes to read,
 * and the driver performs one burst read from FIFO_DATA
 */
ICM42688_Status_t
ICM42688_Get_FIFO_Frame_In_Byte(ICM42688_Handle_t *handle, uint8_t *rawFIFOBuf, uint16_t rawBufSize)
{
    if (!handle || !rawFIFOBuf || rawBufSize == 0U)
        return ICM42688_ERROR;

    // FIFO mode must not be BYPASS
    if (handle->fifo_config.fifo_mode == BYPASS)
        return ICM42688_ERROR;

    // Use FIFO_COUNT_IN_BYTE to know how many bytes for SPI burst read to handle
    if (handle->fifo_config.fifo_count_rec != FIFO_COUNT_IN_BYTE)
        return ICM42688_ERROR;

    // Must re-read the entire FIFO packet
    if (handle->fifo_config.fifo_partial_read_state != FIFO_PARTIAL_READ_DISABLE)
        return ICM42688_ERROR;

    uint16_t fifo_count_in_byte = 0U;

    HAL_StatusTypeDef status = ICM42688_Get_FIFO_Count(handle, &fifo_count_in_byte);
    if (status != HAL_OK)
        return ICM42688_ERROR;

    if ((fifo_count_in_byte == 0U) || (fifo_count_in_byte > rawBufSize))
        return ICM42688_ERROR;

    status = ICM42688_ReadRegs(handle, ICM42688_UB0_FIFO_DATA, rawFIFOBuf, fifo_count_in_byte);
    if (status != HAL_OK)
        return ICM42688_ERROR;

    return ICM42688_OK;
}



/* @brief	Parse one byte FIFO frame.
 * @note	Must call ICM42688_Get_FIFO_Frame_In_Byte before using this function */

// CHECKTHIS !!!!!!!!!!
ICM42688_Status_t
ICM42688_FIFO_Parse_One_Byte_Frame(ICM42688_Handle_t *handle, ICM42688_FIFO_Frame_t *frame,
                                   const uint8_t *FIFO_byteBuf, uint16_t FIFOCountInByte,
                                   uint16_t *currentPos)
{
    if (!handle || !FIFO_byteBuf || !currentPos || !frame || *currentPos >= FIFOCountInByte)
        return ICM42688_ERROR;
    if (*currentPos >= FIFOCountInByte)
        return ICM42688_ERROR;

    uint8_t                header          = FIFO_byteBuf[*currentPos];
    ICM42688_FIFO_Packet_t FIFO_packetType = FIFO_PACKET_INVALID;
    uint8_t                FIFO_packetSize = 0U;

    HAL_StatusTypeDef status =
        ICM42688_Get_FIFO_Packet_Info_From_Header(header, &FIFO_packetType, &FIFO_packetSize);
    if (status != HAL_OK)
        return ICM42688_ERROR;

    if ((FIFO_packetSize == 0U) || (FIFO_packetType == FIFO_PACKET_INVALID))
        return ICM42688_ERROR;

    if ((uint32_t)*currentPos + (uint32_t)FIFO_packetSize > (uint32_t)FIFOCountInByte)
        return ICM42688_ERROR;

    status =
        ICM42688_FIFO_Parse_Frame(handle, frame, FIFO_byteBuf, FIFO_packetSize, FIFO_packetType);
    if (status != HAL_OK)
        return ICM42688_ERROR;

    *currentPos += (uint16_t)FIFO_packetSize;

    return ICM42688_OK;
}