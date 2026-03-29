/*
 * icm42688_fifo.c
 *
 *  Created on: Mar 12, 2026
 *      Author: dobao
 */

#include "imu/features/icm42688_fifo.h"
#include "string.h"

/*==================================================================================
 *	FIFO CONFIG
 *================================================================================== */
HAL_StatusTypeDef
ICM42688_Set_FIFO_Count_Endian(ICM42688_Handle_t *handle, ICM42688_FIFO_Count_Endian_t which_endian)
{
    if (!handle)
        return HAL_ERROR;

    if (((uint8_t)which_endian != 0U && (uint8_t)which_endian != 1U))
        return HAL_ERROR;

    HAL_StatusTypeDef status =
        ICM42688_Update_Reg_Bits(handle, ICM42688_UB0_INTF_CONF0, ICM42688_FIFO_COUNT_ENDIAN_Msk,
                                 ICM42688_FIFO_COUNT_ENDIAN_Val(which_endian));
    if (status != HAL_OK)
        return status;

    handle->fifo_config.fifo_count_endian = which_endian;

    return HAL_OK;
}



HAL_StatusTypeDef
ICM42688_Set_FIFO_Count_Rec(ICM42688_Handle_t *handle, ICM42688_FIFO_Count_Rec_t fifo_count_rec)
{
    if (!handle)
        return HAL_ERROR;

    HAL_StatusTypeDef status =
        ICM42688_Update_Reg_Bits(handle, ICM42688_UB0_INTF_CONF0, ICM42688_FIFO_COUNT_REC_Msk,
                                 ICM42688_FIFO_COUNT_REC_Val(fifo_count_rec));
    if (status != HAL_OK)
        return status;

    handle->fifo_config.fifo_count_rec = fifo_count_rec;

    return HAL_OK;
}



HAL_StatusTypeDef
ICM42688_Set_FIFO_Mode(ICM42688_Handle_t *handle, ICM42688_FIFO_Mode_t mode)
{
    if (!handle)
        return HAL_ERROR;

    if ((uint8_t)mode > (uint8_t)STOP_ON_FULL)
        return HAL_ERROR;

    HAL_StatusTypeDef status = ICM42688_Update_Reg_Bits(
        handle, ICM42688_UB0_FIFO_CONF, ICM42688_FIFO_MODE_Msk, ICM42688_FIFO_MODE_Val(mode));

    if (status != HAL_OK)
        return status;

    handle->fifo_config.fifo_mode = (ICM42688_FIFO_Mode_t)mode;

    return HAL_OK;
}



HAL_StatusTypeDef
ICM42688_Get_FIFO_Mode(ICM42688_Handle_t *handle, ICM42688_FIFO_Mode_t *mode)
{
    if (!handle || !mode)
        return HAL_ERROR;

    uint8_t           reg    = 0U;
    HAL_StatusTypeDef status = ICM42688_ReadReg(handle, ICM42688_UB0_FIFO_CONF, &reg);
    if (status != HAL_OK)
        return status;

    uint8_t rawMode          = (uint8_t)((reg & ICM42688_FIFO_MODE_Msk) >> ICM42688_FIFO_MODE_Pos);
    bool    isModeStopOnFull = (rawMode == 2U) || (rawMode == 3U);

    *mode = (isModeStopOnFull) ? STOP_ON_FULL : (ICM42688_FIFO_Mode_t)rawMode;

    handle->fifo_config.fifo_mode = (ICM42688_FIFO_Mode_t)*mode;

    return HAL_OK;
}



HAL_StatusTypeDef
ICM42688_Set_FIFO_Gyro_Enable(ICM42688_Handle_t *handle, ICM42688_FIFO_GAT_En_t state)
{
    if (!handle)
        return HAL_ERROR;

    HAL_StatusTypeDef status =
        ICM42688_Update_Reg_Bits(handle, ICM42688_UB0_FIFO_CONF1, ICM42688_FIFO_GYRO_EN_Msk,
                                 ICM42688_FIFO_GYRO_EN_Val(state));
    if (status != HAL_OK)
        return status;

    handle->fifo_config.fifo_gyro_state = (ICM42688_FIFO_GAT_En_t)state;

    return HAL_OK;
}



HAL_StatusTypeDef
ICM42688_Set_FIFO_Accel_Enable(ICM42688_Handle_t *handle, ICM42688_FIFO_GAT_En_t state)
{
    if (!handle)
        return HAL_ERROR;

    HAL_StatusTypeDef status =
        ICM42688_Update_Reg_Bits(handle, ICM42688_UB0_FIFO_CONF1, ICM42688_FIFO_ACCEL_EN_Msk,
                                 ICM42688_FIFO_ACCEL_EN_Val(state));
    if (status != HAL_OK)
        return status;

    handle->fifo_config.fifo_accel_state = (ICM42688_FIFO_GAT_En_t)state;

    return HAL_OK;
}



HAL_StatusTypeDef
ICM42688_Set_FIFO_Temp_Enable(ICM42688_Handle_t *handle, ICM42688_FIFO_GAT_En_t state)
{
    if (!handle)
        return HAL_ERROR;

    HAL_StatusTypeDef status =
        ICM42688_Update_Reg_Bits(handle, ICM42688_UB0_FIFO_CONF1, ICM42688_FIFO_TEMP_EN_Msk,
                                 ICM42688_FIFO_TEMP_EN_Val(state));
    if (status != HAL_OK)
        return status;

    handle->fifo_config.fifo_temp_state = (ICM42688_FIFO_GAT_En_t)state;

    return HAL_OK;
}



HAL_StatusTypeDef
ICM42688_Set_FIFO_HIRES_Enable(ICM42688_Handle_t *handle, ICM42688_FIFO_Hires_En_t state)
{
    if (!handle)
        return HAL_ERROR;

    HAL_StatusTypeDef status =
        ICM42688_Update_Reg_Bits(handle, ICM42688_UB0_FIFO_CONF1, ICM42688_FIFO_HIRES_EN_Msk,
                                 ICM42688_FIFO_HIRES_EN_Val(state));
    if (status != HAL_OK)
        return status;

    handle->fifo_config.fifo_hires_state = (ICM42688_FIFO_Hires_En_t)state;

    return HAL_OK;
}



HAL_StatusTypeDef
ICM42688_Set_FIFO_WM_GT_THS(ICM42688_Handle_t *handle, ICM42688_FIFO_WM_Mode_t state)
{
    if (!handle)
        return HAL_ERROR;

    HAL_StatusTypeDef status =
        ICM42688_Update_Reg_Bits(handle, ICM42688_UB0_FIFO_CONF1, ICM42688_FIFO_WM_GT_TH_Msk,
                                 ICM42688_FIFO_WM_GT_TH_Val(state));
    if (status != HAL_OK)
        return status;

    handle->fifo_config.fifo_wm_mode = (ICM42688_FIFO_WM_Mode_t)state;

    return HAL_OK;
}



HAL_StatusTypeDef
ICM42688_Set_FIFO_Resume_Partial_Read(ICM42688_Handle_t *handle, ICM42688_FIFO_Resume_Read_t state)
{
    if (!handle)
        return HAL_ERROR;
    HAL_StatusTypeDef status = ICM42688_Update_Reg_Bits(handle, ICM42688_UB0_FIFO_CONF1,
                                                        ICM42688_FIFO_RESUME_PARTIAL_RD_Msk,
                                                        ICM42688_FIFO_RESUME_PARTIAL_RD_Val(state));
    if (status != HAL_OK)
        return status;

    handle->fifo_config.fifo_partial_read_state = (ICM42688_FIFO_Resume_Read_t)state;

    return HAL_OK;
}



HAL_StatusTypeDef
ICM42688_Set_FIFO_Watermark(ICM42688_Handle_t *handle, uint16_t fifoWatermark)
{
    if (!handle)
        return HAL_ERROR;

    if ((fifoWatermark == 0U) || (fifoWatermark > 0x0FFFU))
        return HAL_ERROR;

    uint8_t fifoLowerWm = (uint8_t)(fifoWatermark & 0x00FFU);
    uint8_t fifoUpperWm = (uint8_t)((fifoWatermark >> 8) & 0x0FU);

    HAL_StatusTypeDef status =
        ICM42688_Update_Reg_Bits(handle, ICM42688_UB0_FIFO_CONF2, ICM42688_FIFO_WM_LOWER_Msk,
                                 ICM42688_FIFO_WM_LOWER_Val(fifoLowerWm));
    if (status != HAL_OK)
        return status;

    status = ICM42688_Update_Reg_Bits(handle, ICM42688_UB0_FIFO_CONF3, ICM42688_FIFO_WM_UPPER_Msk,
                                      ICM42688_FIFO_WM_UPPER_Val(fifoUpperWm));
    if (status != HAL_OK)
        return status;

    handle->fifo_config.fifo_watermark = (uint16_t)fifoWatermark;

    return HAL_OK;
}



HAL_StatusTypeDef
ICM42688_Get_FIFO_Watermark(ICM42688_Handle_t *handle, uint16_t *fifoWatermark)
{
    if (!handle || !fifoWatermark)
        return HAL_ERROR;

    uint8_t           fifoWmBuf[2];
    HAL_StatusTypeDef status = ICM42688_ReadRegs(handle, ICM42688_UB0_FIFO_CONF2, fifoWmBuf, 2);
    if (status != HAL_OK)
        return status;

    *fifoWatermark = (uint16_t)((uint16_t)((fifoWmBuf[1] & ICM42688_FIFO_WM_UPPER_Msk) << 8U) |
                                (uint16_t)((fifoWmBuf[0] & ICM42688_FIFO_WM_LOWER_Msk)));

    handle->fifo_config.fifo_watermark = (uint16_t)*fifoWatermark;

    return HAL_OK;
}



HAL_StatusTypeDef
ICM42688_Get_FIFO_Count(ICM42688_Handle_t *handle, uint16_t *fifoCount)
{
    if (!handle || !fifoCount)
        return HAL_ERROR;

    uint8_t           fifoCountBuf[2];
    HAL_StatusTypeDef status = ICM42688_ReadRegs(handle, ICM42688_UB0_FIFO_COUNTH, fifoCountBuf, 2);
    if (status != HAL_OK)
        return status;

    if (handle->fifo_config.fifo_count_endian == FIFO_COUNT_BIG_ENDIAN) { // Big endian format
        *fifoCount = (uint16_t)(((uint16_t)(fifoCountBuf[0] & 0xFFU) << 8) |
                                ((uint16_t)(fifoCountBuf[1] & 0xFFU)));
    }
    else {
        *fifoCount = (uint16_t)(((uint16_t)(fifoCountBuf[1] & 0xFFU) << 8) |
                                ((uint16_t)(fifoCountBuf[0] & 0xFFU)));
    }

    handle->fifo_config.fifo_count = (uint16_t)*fifoCount;

    return HAL_OK;
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



HAL_StatusTypeDef
ICM42688_Get_FIFO_Packet_Info_From_Header(uint8_t header, ICM42688_FIFO_Packet_t *packetType,
                                          uint8_t *packetSize)
{
    if (!packetType || !packetSize)
        return HAL_ERROR;

    const bool hasMsg = ICM42688_FIFO_Header_Has(header, ICM42688_FIFO_HEADER_MSG_Msk);

    const bool hasAccel = ICM42688_FIFO_Header_Has(header, ICM42688_FIFO_HEADER_ACCEL_Msk);

    const bool hasGyro = ICM42688_FIFO_Header_Has(header, ICM42688_FIFO_HEADER_GYRO_Msk);

    const bool has20bit = ICM42688_FIFO_Header_Has(header, ICM42688_FIFO_HEADER_20_Msk);

    if (hasMsg) {
        *packetType = FIFO_PACKET_INVALID;
        *packetSize = 0U;
        return HAL_ERROR;
    }

    if (!hasAccel && !hasGyro) {
        *packetType = FIFO_PACKET_INVALID;
        *packetSize = 0U;
        return HAL_ERROR;
    }

    // Packet 1: Accel only, 8 bytes
    if (hasAccel && !hasGyro && !has20bit) {
        *packetType = FIFO_PACKET_1;
        *packetSize = 8U;
        return HAL_OK;
    }

    // Packet 2: Gyro only, 8 bytes
    if (hasGyro && !hasAccel && !has20bit) {
        *packetType = FIFO_PACKET_2;
        *packetSize = 8U;
        return HAL_OK;
    }

    // Packet 3: Accel + Gyro 16 bytes
    if (hasAccel && hasGyro && !has20bit) {
        *packetType = FIFO_PACKET_3;
        *packetSize = 16U;
        return HAL_OK;
    }

    // Packet 4: Accel + Gyro + 20 bytes
    if (hasAccel && hasGyro && has20bit) {
        *packetType = FIFO_PACKET_4;
        *packetSize = 20U;
        return HAL_OK;
    }

    *packetType = FIFO_PACKET_INVALID;
    *packetSize = 0U;

    return HAL_ERROR;
}



HAL_StatusTypeDef
ICM42688_FIFO_Parse_Frame(ICM42688_Handle_t *handle, ICM42688_FIFO_Frame_t *FIFO_frame,
                          const uint8_t *FIFO_packet, uint8_t FIFO_packetSize,
                          ICM42688_FIFO_Packet_t FIFO_packetType)
{
    if (!handle || !FIFO_packet || !FIFO_frame)
        return HAL_ERROR;

    // Clean the leftover members in frame to start a known/clean state before the real parsing
    memset(FIFO_frame, 0, sizeof(*FIFO_frame));

    // Write variables into FIFO frame struct
    FIFO_frame->header      = FIFO_packet[0];
    FIFO_frame->packet_type = FIFO_packetType;
    FIFO_frame->packet_size = FIFO_packetSize;

    FIFO_frame->timestamp_fsync_mode = ICM42688_Get_FIFO_TimestampFsync_Mode(FIFO_frame->header);

    FIFO_frame->odr_accel_changed =
        ICM42688_FIFO_Header_Has(FIFO_frame->header, ICM42688_FIFO_HEADER_ODR_ACCEL_Msk);

    FIFO_frame->odr_gyro_changed =
        ICM42688_FIFO_Header_Has(FIFO_frame->header, ICM42688_FIFO_HEADER_ODR_GYRO_Msk);

    // Copy every valid byte in
    for (uint16_t i = 0U; i < FIFO_packetSize; i++) {
        FIFO_frame->raw[i] = FIFO_packet[i];
    }

    /* timestamp/fsync mode:
     * 		00 none
     * 		01 reserved
     * 		10 ODR timestamp
     * 		11 FSYNC time */
    if (FIFO_frame->timestamp_fsync_mode == 1U) {
        return HAL_ERROR;
    }

    FIFO_frame->timestamp_valid =
        (FIFO_frame->timestamp_fsync_mode == 2U) || (FIFO_frame->timestamp_fsync_mode == 3U);
    FIFO_frame->timestamp_fsync_valid = (FIFO_frame->timestamp_fsync_mode == 3U);

    switch (FIFO_frame->packet_type) {
    case FIFO_PACKET_1: {
        FIFO_frame->accel_valid = true;
        FIFO_frame->gyro_valid  = false;
        FIFO_frame->temp_valid  = true;
        FIFO_frame->hires_valid = false;

        FIFO_frame->accel_raw16[0] = ICM42688_Decode_BE16_Signed(FIFO_packet[1], FIFO_packet[2]);

        FIFO_frame->accel_raw16[1] = ICM42688_Decode_BE16_Signed(FIFO_packet[3], FIFO_packet[4]);

        FIFO_frame->accel_raw16[2] = ICM42688_Decode_BE16_Signed(FIFO_packet[5], FIFO_packet[6]);

        if (handle->accel_g_per_lsb > 0.0f) {
            float s                = handle->accel_g_per_lsb;
            FIFO_frame->accel_g[0] = (float)(FIFO_frame->accel_raw16[0] * s);
            FIFO_frame->accel_g[1] = (float)(FIFO_frame->accel_raw16[1] * s);
            FIFO_frame->accel_g[2] = (float)(FIFO_frame->accel_raw16[2] * s);
        }

        FIFO_frame->temp_raw8 = (int8_t)FIFO_packet[7];
        FIFO_frame->temp_c    = ((float)FIFO_frame->temp_raw8 / 2.07f) + 25.0f;

        break;
    }

    case FIFO_PACKET_2: {
        FIFO_frame->accel_valid = false;
        FIFO_frame->gyro_valid  = true;
        FIFO_frame->temp_valid  = true;
        FIFO_frame->hires_valid = false;

        FIFO_frame->gyro_raw16[0] = ICM42688_Decode_BE16_Signed(FIFO_packet[1], FIFO_packet[2]);

        FIFO_frame->gyro_raw16[1] = ICM42688_Decode_BE16_Signed(FIFO_packet[3], FIFO_packet[4]);

        FIFO_frame->gyro_raw16[2] = ICM42688_Decode_BE16_Signed(FIFO_packet[5], FIFO_packet[6]);

        if (handle->gyro_dps_per_lsb > 0.0f) {
            float s                 = handle->gyro_dps_per_lsb;
            FIFO_frame->gyro_dps[0] = (float)(FIFO_frame->gyro_raw16[0] * s);
            FIFO_frame->gyro_dps[1] = (float)(FIFO_frame->gyro_raw16[1] * s);
            FIFO_frame->gyro_dps[2] = (float)(FIFO_frame->gyro_raw16[2] * s);
        }

        FIFO_frame->temp_raw8 = (int8_t)FIFO_packet[7];
        FIFO_frame->temp_c    = ((float)FIFO_frame->temp_raw8 / 2.07f) + 25.0f;

        break;
    }

    case FIFO_PACKET_3: {
        FIFO_frame->accel_valid = true;
        FIFO_frame->gyro_valid  = true;
        FIFO_frame->temp_valid  = true;
        FIFO_frame->hires_valid = false;

        FIFO_frame->accel_raw16[0] = ICM42688_Decode_BE16_Signed(FIFO_packet[1], FIFO_packet[2]);

        FIFO_frame->accel_raw16[1] = ICM42688_Decode_BE16_Signed(FIFO_packet[3], FIFO_packet[4]);

        FIFO_frame->accel_raw16[2] = ICM42688_Decode_BE16_Signed(FIFO_packet[5], FIFO_packet[6]);

        FIFO_frame->gyro_raw16[0] = ICM42688_Decode_BE16_Signed(FIFO_packet[7], FIFO_packet[8]);

        FIFO_frame->gyro_raw16[1] = ICM42688_Decode_BE16_Signed(FIFO_packet[9], FIFO_packet[10]);

        FIFO_frame->gyro_raw16[2] = ICM42688_Decode_BE16_Signed(FIFO_packet[11], FIFO_packet[12]);

        if ((handle->gyro_dps_per_lsb > 0.0f) && (handle->accel_g_per_lsb > 0.0f)) {
            float sa = handle->accel_g_per_lsb;
            float sg = handle->gyro_dps_per_lsb;

            FIFO_frame->accel_g[0] = (float)(FIFO_frame->accel_raw16[0] * sa);
            FIFO_frame->accel_g[1] = (float)(FIFO_frame->accel_raw16[1] * sa);
            FIFO_frame->accel_g[2] = (float)(FIFO_frame->accel_raw16[2] * sa);

            FIFO_frame->gyro_dps[0] = (float)(FIFO_frame->gyro_raw16[0] * sg);
            FIFO_frame->gyro_dps[1] = (float)(FIFO_frame->gyro_raw16[1] * sg);
            FIFO_frame->gyro_dps[2] = (float)(FIFO_frame->gyro_raw16[2] * sg);
        }

        FIFO_frame->temp_raw8 = (int8_t)FIFO_packet[13];
        FIFO_frame->temp_c    = ((float)FIFO_frame->temp_raw8 / 2.07f) + 25.0f;
        FIFO_frame->timestamp = ICM42688_Decode_BE16_Unsigned(FIFO_packet[14], FIFO_packet[15]);

        break;
    }

    case FIFO_PACKET_4: {
        FIFO_frame->accel_valid = true;
        FIFO_frame->gyro_valid  = true;
        FIFO_frame->temp_valid  = true;
        FIFO_frame->hires_valid = true;

        uint32_t accel_x_raw = (uint32_t)(FIFO_packet[1] << 12) | (uint32_t)(FIFO_packet[2] << 4) |
                               (uint32_t)((FIFO_packet[17] >> 4) & 0x0F);

        uint32_t accel_y_raw = (uint32_t)(FIFO_packet[3] << 12) | (uint32_t)(FIFO_packet[4] << 4) |
                               (uint32_t)((FIFO_packet[18] >> 4) & 0x0F);

        uint32_t accel_z_raw = (uint32_t)(FIFO_packet[5] << 12) | (uint32_t)(FIFO_packet[6] << 4) |
                               (uint32_t)((FIFO_packet[19] >> 4) & 0x0F);

        uint32_t gyro_x_raw = (uint32_t)(FIFO_packet[7] << 12) | (uint32_t)(FIFO_packet[8] << 4) |
                              (uint32_t)(FIFO_packet[17] & 0x0F);

        uint32_t gyro_y_raw = (uint32_t)(FIFO_packet[9] << 12) | (uint32_t)(FIFO_packet[10] << 4) |
                              (uint32_t)(FIFO_packet[18] & 0x0F);

        uint32_t gyro_z_raw = (uint32_t)(FIFO_packet[11] << 12) | (uint32_t)(FIFO_packet[12] << 4) |
                              (uint32_t)(FIFO_packet[19] & 0x0F);

        FIFO_frame->accel_raw20[0] = ICM42688_SignExtend20(accel_x_raw);
        FIFO_frame->accel_raw20[1] = ICM42688_SignExtend20(accel_y_raw);
        FIFO_frame->accel_raw20[2] = ICM42688_SignExtend20(accel_z_raw);

        FIFO_frame->gyro_raw20[0] = ICM42688_SignExtend20(gyro_x_raw);
        FIFO_frame->gyro_raw20[1] = ICM42688_SignExtend20(gyro_y_raw);
        FIFO_frame->gyro_raw20[2] = ICM42688_SignExtend20(gyro_z_raw);

        FIFO_frame->temp_raw16 = ICM42688_Decode_BE16_Signed(FIFO_packet[13], FIFO_packet[14]);

        FIFO_frame->temp_c = (float)((FIFO_frame->temp_raw16) / 132.48f) + 25.0f;

        FIFO_frame->timestamp = ICM42688_Decode_BE16_Unsigned(FIFO_packet[15], FIFO_packet[16]);

        const float accel_g_per_lsb_p4  = 1.0f / 8192.0f;
        const float gyro_dps_per_lsb_p4 = 1.0f / 131.0f;

        FIFO_frame->accel_g[0] = accel_g_per_lsb_p4 * (FIFO_frame->accel_raw20[0]);
        FIFO_frame->accel_g[1] = accel_g_per_lsb_p4 * (FIFO_frame->accel_raw20[1]);
        FIFO_frame->accel_g[2] = accel_g_per_lsb_p4 * (FIFO_frame->accel_raw20[2]);

        FIFO_frame->gyro_dps[0] = gyro_dps_per_lsb_p4 * (FIFO_frame->gyro_raw20[0]);
        FIFO_frame->gyro_dps[1] = gyro_dps_per_lsb_p4 * (FIFO_frame->gyro_raw20[1]);
        FIFO_frame->gyro_dps[2] = gyro_dps_per_lsb_p4 * (FIFO_frame->gyro_raw20[2]);

        break;
    }

    default:
        return HAL_ERROR;
    }
    return HAL_OK;
}

HAL_StatusTypeDef
ICM42688_Get_FIFO_Frame_In_Record(ICM42688_Handle_t *handle, ICM42688_FIFO_Frame_t *FIFO_frame)
{
    if (!handle || !FIFO_frame)
        return HAL_ERROR;

    // FIFO mode must not be BYPASS
    if (handle->fifo_config.fifo_mode == BYPASS)
        return HAL_ERROR;

    // Use FIFO_COUNT_IN_RECORD because this function must read header first before knowing packet
    // size
    if (handle->fifo_config.fifo_count_rec != FIFO_COUNT_IN_RECORD)
        return HAL_ERROR;

    // Must enable resume partial FIFO read
    if (handle->fifo_config.fifo_partial_read_state != FIFO_PARTIAL_READ_ENABLE)
        return HAL_ERROR;

    // Get total available number of packets
    uint16_t          fifoCountInRecord = 0U;
    HAL_StatusTypeDef status            = ICM42688_Get_FIFO_Count(handle, &fifoCountInRecord);
    if (status != HAL_OK)
        return status;
    if (fifoCountInRecord < 1U)
        return HAL_ERROR;

    // Extract Packet Type & Packet Size from header by reading the first byte from register
    // FIFO_DATA
    uint8_t header = 0U;
    status         = ICM42688_ReadRegs(handle, ICM42688_UB0_FIFO_DATA, &header, 1);
    if (status != HAL_OK)
        return status;

    ICM42688_FIFO_Packet_t FIFO_packetType = FIFO_PACKET_INVALID;
    uint8_t                FIFO_packetSize = 0U;

    status = ICM42688_Get_FIFO_Packet_Info_From_Header(header, &FIFO_packetType, &FIFO_packetSize);
    if (status != HAL_OK)
        return status;
    if ((FIFO_packetSize == 0U) || (FIFO_packetType == FIFO_PACKET_INVALID))
        return HAL_ERROR;

    // Start resuming reading the remaining bytes in that packet
    uint8_t FIFO_packet[20] = {0};
    FIFO_packet[0]          = header;
    status                  = ICM42688_ReadRegs(handle, ICM42688_UB0_FIFO_DATA, &FIFO_packet[1],
                                                (uint16_t)(FIFO_packetSize - 1U));
    if (status != HAL_OK)
        return status;

    // Start parsing
    status = ICM42688_FIFO_Parse_Frame(handle, FIFO_frame, FIFO_packet, FIFO_packetSize,
                                       FIFO_packetType);
    if (status != HAL_OK)
        return status;

    return HAL_OK;
}

/* @brief	Read all available FIFO bytes in one SPI burst
 * @note 	This function is valid only:
 * 			FIFO is enabled NOT BYPASS
 * 			FIFO count is configured to report in BYTES
 * 			Partial FIFO read is DISABLED
 * 		In this mode, FIFO_COUNT gives the total number of bytes to read,
 * 		and the driver performs one burst read from FIFO_DATA */
HAL_StatusTypeDef
ICM42688_Get_FIFO_Frame_In_Byte(ICM42688_Handle_t *handle, uint8_t *rawFIFOBuf, uint16_t rawBufSize,
                                uint16_t *FIFOCountInByte)
{
    if (!handle || !rawFIFOBuf || rawBufSize == 0U || !FIFOCountInByte)
        return HAL_ERROR;

    // FIFO mode must not be BYPASS
    if (handle->fifo_config.fifo_mode == BYPASS)
        return HAL_ERROR;

    // Use FIFO_COUNT_IN_BYTE to know how many bytes for SPI burst read to handle
    if (handle->fifo_config.fifo_count_rec != FIFO_COUNT_IN_BYTE)
        return HAL_ERROR;

    // Must re-read the entire FIFO packet
    if (handle->fifo_config.fifo_partial_read_state != FIFO_PARTIAL_READ_DISABLE)
        return HAL_ERROR;

    *FIFOCountInByte = 0U;

    HAL_StatusTypeDef status = ICM42688_Get_FIFO_Count(handle, FIFOCountInByte);
    if (status != HAL_OK)
        return status;

    if ((*FIFOCountInByte == 0U) || (*FIFOCountInByte > rawBufSize))
        return HAL_ERROR;

    status = ICM42688_ReadRegs(handle, ICM42688_UB0_FIFO_DATA, rawFIFOBuf, *FIFOCountInByte);
    if (status != HAL_OK)
        return status;

    return HAL_OK;
}

/* @brief	Parse one byte FIFO frame.
 * @note	Must call ICM42688_Get_FIFO_Frame_In_Byte before using this function */
HAL_StatusTypeDef
ICM42688_FIFO_Parse_One_Byte_Frame(ICM42688_Handle_t *handle, ICM42688_FIFO_Frame_t *FIFO_frame,
                                   const uint8_t *FIFO_byteBuf, uint16_t FIFOCountInByte,
                                   uint16_t *currentPos)
{
    if (!handle || !FIFO_byteBuf || !currentPos || !FIFO_frame || *currentPos >= FIFOCountInByte)
        return HAL_ERROR;
    if (*currentPos >= FIFOCountInByte)
        return HAL_ERROR;

    uint8_t                header          = FIFO_byteBuf[*currentPos];
    ICM42688_FIFO_Packet_t FIFO_packetType = FIFO_PACKET_INVALID;
    uint8_t                FIFO_packetSize = 0U;
    HAL_StatusTypeDef      status =
        ICM42688_Get_FIFO_Packet_Info_From_Header(header, &FIFO_packetType, &FIFO_packetSize);
    if (status != HAL_OK)
        return status;
    if ((FIFO_packetSize == 0U) || (FIFO_packetType == FIFO_PACKET_INVALID))
        return HAL_ERROR;
    if ((uint32_t)*currentPos + (uint32_t)FIFO_packetSize > (uint32_t)FIFOCountInByte)
        return HAL_ERROR;

    status = ICM42688_FIFO_Parse_Frame(handle, FIFO_frame, FIFO_byteBuf, FIFO_packetSize,
                                       FIFO_packetType);
    if (status != HAL_OK)
        return status;

    *currentPos += (uint16_t)FIFO_packetSize;

    return HAL_OK;
}
