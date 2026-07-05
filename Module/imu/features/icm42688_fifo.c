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
bool
ICM42688_Set_FIFO_Count_Endian(ICM42688_Handle_t *handle, ICM42688_FIFO_Count_Endian_t countEndian)
{
    if (!handle)
        return false;

    if (((uint8_t)countEndian != 0U && (uint8_t)countEndian != 1U))
        return false;

    bool _status = ICM42688_Update_Reg_Bits(handle, ICM42688_UB0_INTF_CONF0, ICM42688_FIFO_COUNT_ENDIAN_Msk,
                                            ICM42688_FIFO_COUNT_ENDIAN_Val(countEndian));
    if (!_status)
        return false;

    handle->fifo_config.fifo_count_endian = countEndian;

    return true;
}



/**
 * @brief   Set a desired count record type
 * @param   handle      Pointer to ICM42688 Handle struct
 * @param   countRecord Selects whether FIFO_COUNT reports bytes or records
 */
bool
ICM42688_Set_FIFO_Count_Rec(ICM42688_Handle_t *handle, ICM42688_FIFO_Count_Rec_t countRecord)
{
    if (!handle)
        return false;

    if (((uint8_t)countRecord != 0U) && ((uint8_t)countRecord != 1U))
        return false;

    bool _status = ICM42688_Update_Reg_Bits(handle, ICM42688_UB0_INTF_CONF0, ICM42688_FIFO_COUNT_REC_Msk,
                                            ICM42688_FIFO_COUNT_REC_Val(countRecord));
    if (!_status)
        return false;

    handle->fifo_config.fifo_count_rec = countRecord;

    return true;
}



/**
 * @brief   Set a desired FIFO mode
 * @param   handle  Pointer to ICM42688 Handle struct
 * @param   mode    Desired FIFO working mode
 */
bool
ICM42688_Set_FIFO_Mode(ICM42688_Handle_t *handle, ICM42688_FIFO_Mode_t mode)
{
    if (!handle)
        return false;

    if ((uint8_t)mode > (uint8_t)STOP_ON_FULL)
        return false;

    bool _status =
        ICM42688_Update_Reg_Bits(handle, ICM42688_UB0_FIFO_CONF, ICM42688_FIFO_MODE_Msk, ICM42688_FIFO_MODE_Val(mode));

    if (!_status)
        return false;

    handle->fifo_config.fifo_mode = (ICM42688_FIFO_Mode_t)mode;

    return true;
}



/**
 * @brief   Read the current FIFO working mode
 * @param   handle  Pointer to ICM42688 Handle struct
 * @param   mode    Pointer to a variable that stores the actual FIFO working mode
 */
bool
ICM42688_Get_FIFO_Mode(ICM42688_Handle_t *handle, ICM42688_FIFO_Mode_t *mode)
{
    if (!handle || !mode)
        return false;

    uint8_t _reg    = 0U;
    bool    _status = ICM42688_ReadReg(handle, ICM42688_UB0_FIFO_CONF, &_reg);
    if (!_status)
        return false;

    uint8_t _raw_mode             = (uint8_t)((_reg & ICM42688_FIFO_MODE_Msk) >> ICM42688_FIFO_MODE_Pos);
    bool    _is_mode_stop_on_full = (_raw_mode == 2U) || (_raw_mode == 3U);

    *mode = (_is_mode_stop_on_full) ? STOP_ON_FULL : (ICM42688_FIFO_Mode_t)_raw_mode;

    handle->fifo_config.fifo_mode = (ICM42688_FIFO_Mode_t)*mode;

    return true;
}



/**
 * @brief   Enable FIFO for Gyro sensor
 * @param   handle  Pointer to ICM42688 Handle struct
 * @param   state   Enable or Disable
 */
bool
ICM42688_Set_FIFO_Gyro_Enable(ICM42688_Handle_t *handle, ICM42688_FIFO_GAT_En_t state)
{
    if (!handle)
        return false;

    if (((uint8_t)state != 0U) && ((uint8_t)state != 1U))
        return false;

    bool _status = ICM42688_Update_Reg_Bits(handle, ICM42688_UB0_FIFO_CONF1, ICM42688_FIFO_GYRO_EN_Msk,
                                            ICM42688_FIFO_GYRO_EN_Val(state));
    if (!_status)
        return false;

    handle->fifo_config.fifo_gyro_state = (ICM42688_FIFO_GAT_En_t)state;

    return true;
}



/**
 * @brief   Enable FIFO for Accel sensor
 * @param   handle      Pointer to ICM42688 Handle struct
 * @param   state       Enable or Disable
 */
bool
ICM42688_Set_FIFO_Accel_Enable(ICM42688_Handle_t *handle, ICM42688_FIFO_GAT_En_t state)
{
    if (!handle)
        return false;

    if (((uint8_t)state != 0U) && ((uint8_t)state != 1U))
        return false;

    bool _status = ICM42688_Update_Reg_Bits(handle, ICM42688_UB0_FIFO_CONF1, ICM42688_FIFO_ACCEL_EN_Msk,
                                            ICM42688_FIFO_ACCEL_EN_Val(state));
    if (!_status)
        return false;

    handle->fifo_config.fifo_accel_state = (ICM42688_FIFO_GAT_En_t)state;

    return true;
}



/**
 * @brief   Enable FIFO for Temperature sensor
 * @param   handle      Pointer to ICM42688 Handle struct
 * @param   state       Enable or Disable
 */
bool
ICM42688_Set_FIFO_Temp_Enable(ICM42688_Handle_t *handle, ICM42688_FIFO_GAT_En_t state)
{
    if (!handle)
        return false;

    if (((uint8_t)state != 0U) && ((uint8_t)state != 1U))
        return false;

    bool _status = ICM42688_Update_Reg_Bits(handle, ICM42688_UB0_FIFO_CONF1, ICM42688_FIFO_TEMP_EN_Msk,
                                            ICM42688_FIFO_TEMP_EN_Val(state));
    if (!_status)
        return false;

    handle->fifo_config.fifo_temp_state = (ICM42688_FIFO_GAT_En_t)state;

    return true;
}



/**
 * @brief   Enable high-resolution data / more bit data for Gyro, Accel and Temperature
 * @param   handle      Pointer to ICM42688 Handle struct
 * @param   state       Enable or Disable
 */
bool
ICM42688_Set_FIFO_HIRES_Enable(ICM42688_Handle_t *handle, ICM42688_FIFO_Hires_En_t state)
{
    if (!handle)
        return false;

    if (((uint8_t)state != 0U) && ((uint8_t)state != 1U))
        return false;

    bool _status = ICM42688_Update_Reg_Bits(handle, ICM42688_UB0_FIFO_CONF1, ICM42688_FIFO_HIRES_EN_Msk,
                                            ICM42688_FIFO_HIRES_EN_Val(state));
    if (!_status)
        return false;

    handle->fifo_config.fifo_hires_state = (ICM42688_FIFO_Hires_En_t)state;

    return true;
}



/**
 * @brief   Set "FIFO Watermark Greater than Threshold" oneshot or repeat interrupt mode
 * @param   handle  Pointer to ICM42688 Handle struct
 * @param   state   Oneshot or repeat interrupt
 */
bool
ICM42688_Set_FIFO_WM_GT_THS(ICM42688_Handle_t *handle, ICM42688_FIFO_WM_Mode_t state)
{
    if (!handle)
        return false;

    if (((uint8_t)state != 0U) && ((uint8_t)state != 1U))
        return false;

    bool _status = ICM42688_Update_Reg_Bits(handle, ICM42688_UB0_FIFO_CONF1, ICM42688_FIFO_WM_GT_TH_Msk,
                                            ICM42688_FIFO_WM_GT_TH_Val(state));
    if (!_status)
        return false;

    handle->fifo_config.fifo_wm_mode = (ICM42688_FIFO_WM_Mode_t)state;

    return true;
}



/**
 * @brief   Set "FIFO Resume Partial Read" mode
 * @param   handle  Pointer to ICM42688 Handle struct
 * @param   state   Restart each FIFO read from the beginning or resume from the last read point
 */
bool
ICM42688_Set_FIFO_Resume_Partial_Read(ICM42688_Handle_t *handle, ICM42688_FIFO_Resume_Read_t state)
{
    if (!handle)
        return false;

    if (((uint8_t)state != 0U) && ((uint8_t)state != 1U))
        return false;

    bool _status = ICM42688_Update_Reg_Bits(handle, ICM42688_UB0_FIFO_CONF1, ICM42688_FIFO_RESUME_PARTIAL_RD_Msk,
                                            ICM42688_FIFO_RESUME_PARTIAL_RD_Val(state));
    if (!_status)
        return false;

    handle->fifo_config.fifo_partial_read_state = (ICM42688_FIFO_Resume_Read_t)state;

    return true;
}



/**
 * @brief   Set a desired watermark for FIFO
 * @param   handle          Pointer to ICM42688 Handle struct
 * @param   fifoWatermark   Desired watermark level
 */
bool
ICM42688_Set_FIFO_Watermark(ICM42688_Handle_t *handle, uint16_t fifoWatermark)
{
    if (!handle)
        return false;

    if ((fifoWatermark == 0U) || (fifoWatermark > 0x0FFFU))
        return false;

    // FIFO watermark is a 12-bit value: FIFO_CONF2 stores bits[7:0], FIFO_CONF3 stores bits[11:8].
    uint8_t _fifo_lower_wm = (uint8_t)(fifoWatermark & 0x00FFU);
    uint8_t _fifo_upper_wm = (uint8_t)((fifoWatermark >> 8) & 0x0FU);

    bool _status = ICM42688_Update_Reg_Bits(handle, ICM42688_UB0_FIFO_CONF2, ICM42688_FIFO_WM_LOWER_Msk,
                                            ICM42688_FIFO_WM_LOWER_Val(_fifo_lower_wm));
    if (!_status)
        return false;

    _status = ICM42688_Update_Reg_Bits(handle, ICM42688_UB0_FIFO_CONF3, ICM42688_FIFO_WM_UPPER_Msk,
                                       ICM42688_FIFO_WM_UPPER_Val(_fifo_upper_wm));
    if (!_status)
        return false;

    handle->fifo_config.fifo_watermark = (uint16_t)fifoWatermark;

    return true;
}



/**
 * @brief   Read the current FIFO watermark level
 * @param   handle          Pointer to ICM42688 Handle struct
 * @param   fifoWatermark   Pointer to a variable that stores the returned watermark level
 */
bool
ICM42688_Get_FIFO_Watermark(ICM42688_Handle_t *handle, uint16_t *fifoWatermark)
{
    if (!handle || !fifoWatermark)
        return false;

    uint8_t _fifo_wm_buf[2];
    bool    _status = ICM42688_ReadRegs(handle, ICM42688_UB0_FIFO_CONF2, _fifo_wm_buf, 2);
    if (!_status)
        return false;

    *fifoWatermark = (uint16_t)((uint16_t)((_fifo_wm_buf[1] & ICM42688_FIFO_WM_UPPER_Msk) << 8U) |
                                (uint16_t)((_fifo_wm_buf[0] & ICM42688_FIFO_WM_LOWER_Msk)));

    handle->fifo_config.fifo_watermark = (uint16_t)*fifoWatermark;

    return true;
}



/**
 * @brief   Read the current FIFO byte or record count from the sensor
 * @param   handle      Pointer to ICM42688 Handle struct
 * @param   fifoCount   Pointer to a variable that stores the decoded byte or record count
 */
bool
ICM42688_Get_FIFO_Count(ICM42688_Handle_t *handle, uint16_t *fifoCount)
{
    if (!handle || !fifoCount)
        return false;

    uint8_t _fifo_count_buf[2];
    bool    _status = ICM42688_ReadRegs(handle, ICM42688_UB0_FIFO_COUNTH, _fifo_count_buf, 2);
    if (!_status)
        return false;

    // FIFO_COUNTH is the full high byte of FIFO_COUNT; only FIFO watermark upper bits are nibble-sized.
    if (handle->fifo_config.fifo_count_endian == FIFO_COUNT_BIG_ENDIAN) {
        *fifoCount = (uint16_t)(((uint16_t)_fifo_count_buf[0] << 8) | ((uint16_t)_fifo_count_buf[1]));
    }
    else {
        *fifoCount = (uint16_t)(((uint16_t)_fifo_count_buf[1] << 8) | ((uint16_t)_fifo_count_buf[0]));
    }

    // In byte-count mode, FIFO_COUNT cannot exceed the 2 KB FIFO depth.
    if ((handle->fifo_config.fifo_count_rec == FIFO_COUNT_IN_BYTE) && (*fifoCount > 2048U)) {
        return false;
    }

    handle->fifo_config.fifo_count = (uint16_t)*fifoCount;

    return true;
}



static inline bool
ICM42688_FIFO_Header_Has(uint8_t header, uint8_t mask)
{
    return ((header & mask) != 0U);
}



static inline uint8_t
ICM42688_Get_FIFO_TimestampFsync_Mode(uint8_t header)
{
    return (uint8_t)((header & ICM42688_FIFO_HEADER_TIMESTAMP_FSYNC_Msk) >> ICM42688_FIFO_HEADER_TIMESTAMP_FSYNC_Pos);
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
 * @param   inputHeader     FIFO header byte read from FIFO_DATA
 * @param   packetType      Pointer to an output variable that stores the FIFO packet type from
 *                          decoding FIFO header
 * @param   packetSize      Pointer to an output variable that stores the FIFO packet size based on
 *                          FIFO packet type
 */
bool
ICM42688_Get_FIFO_Packet_Info_From_Header(uint8_t inputHeader, ICM42688_FIFO_Packet_t *packetType, uint8_t *packetSize)
{
    if (!packetType || !packetSize)
        return false;

    const bool _has_msg   = ICM42688_FIFO_Header_Has(inputHeader, ICM42688_FIFO_HEADER_MSG_Msk);
    const bool _has_accel = ICM42688_FIFO_Header_Has(inputHeader, ICM42688_FIFO_HEADER_ACCEL_Msk);
    const bool _has_gyro  = ICM42688_FIFO_Header_Has(inputHeader, ICM42688_FIFO_HEADER_GYRO_Msk);
    const bool _has_20bit = ICM42688_FIFO_Header_Has(inputHeader, ICM42688_FIFO_HEADER_20_Msk);

    if (_has_msg) {
        *packetType = FIFO_PACKET_INVALID;
        *packetSize = 0U;
        return false;
    }

    // A FIFO data packet must contain accel, gyro, or both. Message packets are rejected above.
    if (!_has_accel && !_has_gyro) {
        *packetType = FIFO_PACKET_INVALID;
        *packetSize = 0U;
        return false;
    }

    // Packet 1: Accel only, 8 bytes
    if (_has_accel && !_has_gyro && !_has_20bit) {
        *packetType = FIFO_PACKET_1;
        *packetSize = 8U;
        return true;
    }

    // Packet 2: Gyro only, 8 bytes
    if (_has_gyro && !_has_accel && !_has_20bit) {
        *packetType = FIFO_PACKET_2;
        *packetSize = 8U;
        return true;
    }

    // Packet 3: Accel + Gyro, 16 bytes
    if (_has_accel && _has_gyro && !_has_20bit) {
        *packetType = FIFO_PACKET_3;
        *packetSize = 16U;
        return true;
    }

    // Packet 4: Accel + Gyro + high-resolution extension, 20 bytes
    if (_has_accel && _has_gyro && _has_20bit) {
        *packetType = FIFO_PACKET_4;
        *packetSize = 20U;
        return true;
    }

    // No valid info from FIFO header
    *packetType = FIFO_PACKET_INVALID;
    *packetSize = 0U;

    return false;
}



/**
 * @brief   Parse one FIFO packet and fill the decoded fields in @p frame
 * @param   handle      Pointer to ICM42688 Handle struct
 * @param   frame       Pointer to FIFO frame that stores packet metadata and decoded data
 * @param   data        Pointer to the FIFO packet buffer read consecutively from FIFO_DATA
 *                      register
 * @param   packetType  FIFO packet type decoded from the header
 * @param   packetSize  FIFO packet size decoded from the header
 */
bool
ICM42688_FIFO_Parse_Frame(ICM42688_Handle_t *handle, ICM42688_FIFO_Frame_t *frame, const uint8_t *data,
                          uint8_t packetSize, ICM42688_FIFO_Packet_t packetType)
{
    if (!handle || !data || !frame)
        return false;

    uint8_t _expected_size = 0U;

    switch (packetType) {
        case FIFO_PACKET_1:
        case FIFO_PACKET_2:
            _expected_size = 8U;
            break;

        case FIFO_PACKET_3:
            _expected_size = 16U;
            break;

        case FIFO_PACKET_4:
            _expected_size = 20U;
            break;

        default:
            return false;
    }

    if ((_expected_size != packetSize) || (packetSize > sizeof(frame->raw)))
        return false;

    // Start from a known state so fields from a previous packet cannot leak into this frame.
    memset(frame, 0, sizeof(*frame));

    // Store packet metadata before decoding payload fields.
    frame->header      = data[0];
    frame->packet_type = packetType;
    frame->packet_size = packetSize;

    frame->timestamp_fsync_mode = ICM42688_Get_FIFO_TimestampFsync_Mode(frame->header);

    frame->odr_accel_changed = ICM42688_FIFO_Header_Has(frame->header, ICM42688_FIFO_HEADER_ODR_ACCEL_Msk);

    frame->odr_gyro_changed = ICM42688_FIFO_Header_Has(frame->header, ICM42688_FIFO_HEADER_ODR_GYRO_Msk);

    // Keep a copy of the raw packet for debugging and downstream inspection.
    for (uint16_t _i = 0U; _i < packetSize; _i++) {
        frame->raw[_i] = data[_i];
    }

    /* Timestamp/FSYNC mode:
     *      00: none
     *      01: reserved
     *      10: ODR timestamp
     *      11: FSYNC time */
    if (frame->timestamp_fsync_mode == 1U) {
        return false;
    }

    frame->timestamp_valid = (frame->timestamp_fsync_mode == 2U) || (frame->timestamp_fsync_mode == 3U);

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

            // Convert raw accel to g when the accel scale factor has already been configured.
            if (handle->accel_g_per_lsb > 0.0f) {
                float _s = handle->accel_g_per_lsb;

                frame->gat_scaled.accel_g[0] = (float)(frame->accel_raw16[0] * _s);
                frame->gat_scaled.accel_g[1] = (float)(frame->accel_raw16[1] * _s);
                frame->gat_scaled.accel_g[2] = (float)(frame->accel_raw16[2] * _s);
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

            // Convert raw gyro to dps when the gyro scale factor has already been configured.
            if (handle->gyro_dps_per_lsb > 0.0f) {
                float _s = handle->gyro_dps_per_lsb;

                frame->gat_scaled.gyro_dps[0] = (float)(frame->gyro_raw16[0] * _s);
                frame->gat_scaled.gyro_dps[1] = (float)(frame->gyro_raw16[1] * _s);
                frame->gat_scaled.gyro_dps[2] = (float)(frame->gyro_raw16[2] * _s);
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

            // Convert raw accel and gyro to g and dps when both scale factors are configured.
            if ((handle->gyro_dps_per_lsb > 0.0f) && (handle->accel_g_per_lsb > 0.0f)) {
                float _sa = handle->accel_g_per_lsb;
                float _sg = handle->gyro_dps_per_lsb;

                frame->gat_scaled.accel_g[0] = (float)(frame->accel_raw16[0] * _sa);
                frame->gat_scaled.accel_g[1] = (float)(frame->accel_raw16[1] * _sa);
                frame->gat_scaled.accel_g[2] = (float)(frame->accel_raw16[2] * _sa);

                frame->gat_scaled.gyro_dps[0] = (float)(frame->gyro_raw16[0] * _sg);
                frame->gat_scaled.gyro_dps[1] = (float)(frame->gyro_raw16[1] * _sg);
                frame->gat_scaled.gyro_dps[2] = (float)(frame->gyro_raw16[2] * _sg);
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

            uint32_t _accel_x_raw =
                (uint32_t)(data[1] << 12) | (uint32_t)(data[2] << 4) | (uint32_t)((data[17] >> 4) & 0x0F);

            uint32_t _accel_y_raw =
                (uint32_t)(data[3] << 12) | (uint32_t)(data[4] << 4) | (uint32_t)((data[18] >> 4) & 0x0F);

            uint32_t _accel_z_raw =
                (uint32_t)(data[5] << 12) | (uint32_t)(data[6] << 4) | (uint32_t)((data[19] >> 4) & 0x0F);

            uint32_t _gyro_x_raw = (uint32_t)(data[7] << 12) | (uint32_t)(data[8] << 4) | (uint32_t)(data[17] & 0x0F);

            uint32_t _gyro_y_raw = (uint32_t)(data[9] << 12) | (uint32_t)(data[10] << 4) | (uint32_t)(data[18] & 0x0F);

            uint32_t _gyro_z_raw = (uint32_t)(data[11] << 12) | (uint32_t)(data[12] << 4) | (uint32_t)(data[19] & 0x0F);

            frame->accel_raw20[0] = ICM42688_SignExtend20(_accel_x_raw);
            frame->accel_raw20[1] = ICM42688_SignExtend20(_accel_y_raw);
            frame->accel_raw20[2] = ICM42688_SignExtend20(_accel_z_raw);

            frame->gyro_raw20[0] = ICM42688_SignExtend20(_gyro_x_raw);
            frame->gyro_raw20[1] = ICM42688_SignExtend20(_gyro_y_raw);
            frame->gyro_raw20[2] = ICM42688_SignExtend20(_gyro_z_raw);

            // Packet 4 high-resolution samples use fixed 20-bit scale factors.
            const float _accel_g_per_lsb_p4  = 1.0f / 8192.0f;
            const float _gyro_dps_per_lsb_p4 = 1.0f / 131.0f;

            frame->gat_scaled.accel_g[0] = _accel_g_per_lsb_p4 * (frame->accel_raw20[0]);
            frame->gat_scaled.accel_g[1] = _accel_g_per_lsb_p4 * (frame->accel_raw20[1]);
            frame->gat_scaled.accel_g[2] = _accel_g_per_lsb_p4 * (frame->accel_raw20[2]);

            frame->gat_scaled.gyro_dps[0] = _gyro_dps_per_lsb_p4 * (frame->gyro_raw20[0]);
            frame->gat_scaled.gyro_dps[1] = _gyro_dps_per_lsb_p4 * (frame->gyro_raw20[1]);
            frame->gat_scaled.gyro_dps[2] = _gyro_dps_per_lsb_p4 * (frame->gyro_raw20[2]);

            frame->temp_raw16        = ICM42688_Decode_BE16_Signed(data[13], data[14]);
            frame->gat_scaled.temp_c = (float)((frame->temp_raw16) / 132.48f) + 25.0f;

            frame->timestamp = ICM42688_Decode_BE16_Unsigned(data[15], data[16]);

            break;
        }

        default:
            return false;
    }

    return true;
}



/**
 * @brief   Read one FIFO frame when FIFO_COUNT is configured in record mode
 * @param   handle  Pointer to ICM42688 Handle struct
 * @param   frame   Pointer to an output ICM42688 FIFO frame struct
 */
bool
ICM42688_Get_FIFO_Frame_In_Record(ICM42688_Handle_t *handle, ICM42688_FIFO_Frame_t *frame)
{
    if (!handle || !frame)
        return false;

    // FIFO mode must not be BYPASS.
    if (handle->fifo_config.fifo_mode == BYPASS)
        return false;

    // Record mode reports how many complete FIFO packets are available.
    if (handle->fifo_config.fifo_count_rec != FIFO_COUNT_IN_RECORD)
        return false;

    // Partial-read resume is required because this path reads the header first, then the remaining packet bytes.
    if (handle->fifo_config.fifo_partial_read_state != FIFO_PARTIAL_READ_ENABLE)
        return false;

    // Get the total available number of packets/records.
    uint16_t _fifo_count_in_record = 0U;

    bool _status = ICM42688_Get_FIFO_Count(handle, &_fifo_count_in_record);
    if (!_status)
        return false;

    if (_fifo_count_in_record < 1U)
        return false;

    // Read the packet header first so packet type and size can be decoded.
    uint8_t                _header           = 0U;
    ICM42688_FIFO_Packet_t _fifo_packet_type = FIFO_PACKET_INVALID;
    uint8_t                _fifo_packet_size = 0U;

    // FIFO_DATA returns the next unread byte, which is the packet header in record mode.
    _status = ICM42688_ReadRegs(handle, ICM42688_UB0_FIFO_DATA, &_header, 1);
    if (!_status)
        return false;

    _status = ICM42688_Get_FIFO_Packet_Info_From_Header(_header, &_fifo_packet_type, &_fifo_packet_size);
    if (!_status)
        return false;

    if ((_fifo_packet_size == 0U) || (_fifo_packet_type == FIFO_PACKET_INVALID))
        return false;

    // Resume reading the remaining bytes in this packet.
    uint8_t _fifo_data[20] = {0};
    _fifo_data[0]          = _header;
    _status = ICM42688_ReadRegs(handle, ICM42688_UB0_FIFO_DATA, &_fifo_data[1], (uint16_t)(_fifo_packet_size - 1U));
    if (!_status)
        return false;

    // Decode the packet into the output frame.
    _status = ICM42688_FIFO_Parse_Frame(handle, frame, _fifo_data, _fifo_packet_size, _fifo_packet_type);
    if (!_status)
        return false;

    return true;
}



/**
 * @brief   Read all available FIFO bytes in one SPI burst
 * @note    This function is valid only when:
 *          FIFO is enabled, NOT BYPASS
 *          FIFO count is configured to report in BYTEs
 *          PARTIAL FIFO READ is DISABLED
 * In this mode, FIFO_COUNT gives the total number of bytes to read,
 * and the driver performs one burst read from FIFO_DATA
 * @param   rawBuf      Raw pointer to raw FIFO buffer
 * @param   rawSize     Capacity of @p rawBuf in bytes
 */
bool
ICM42688_Get_FIFO_Frame_In_Byte(ICM42688_Handle_t *handle, uint8_t *rawBuf, uint16_t rawSize)
{
    if (!handle || !rawBuf || rawSize == 0U)
        return false;

    // FIFO mode must not be BYPASS.
    if (handle->fifo_config.fifo_mode == BYPASS)
        return false;

    // Byte-count mode reports the exact number of FIFO_DATA bytes to burst read.
    if (handle->fifo_config.fifo_count_rec != FIFO_COUNT_IN_BYTE)
        return false;

    // Partial-read resume must be disabled because this path drains the FIFO in one burst.
    if (handle->fifo_config.fifo_partial_read_state != FIFO_PARTIAL_READ_DISABLE)
        return false;

    uint16_t _fifo_count_in_byte = 0U;

    bool _status = ICM42688_Get_FIFO_Count(handle, &_fifo_count_in_byte);
    if (!_status)
        return false;

    if ((_fifo_count_in_byte == 0U) || (_fifo_count_in_byte > rawSize))
        return false;

    _status = ICM42688_ReadRegs(handle, ICM42688_UB0_FIFO_DATA, rawBuf, _fifo_count_in_byte);
    if (!_status)
        return false;

    return true;
}



/**
 * @brief   Flush FIFO storage to reset FIFO_COUNT_BYTE/RECORD back to 0
 *          This function should be called after a successful init or before starting a clean capture window
 */
bool
ICM42688_FIFO_Flush(ICM42688_Handle_t *handle, bool enable)
{
    if (!handle)
        return false;

    uint8_t _enable_val = enable ? 1U : 0U;
    bool    _status     = ICM42688_Update_Reg_Bits(handle, ICM42688_UB0_SIGNAL_PATH_RST, ICM42688_FIFO_FLUSH_Msk,
                                                   ICM42688_FIFO_FLUSH_Val(_enable_val));

    if (!_status)
        return false;

    return true;
}



/**
 * @brief   Parse one FIFO frame from a byte buffer previously read from FIFO_DATA
 * @note    Call ICM42688_Get_FIFO_Frame_In_Byte() before using this function.
 * @param   handle          Pointer to an ICM42688 Handle struct
 * @param   frame           Pointer to an output ICM42688 FIFO frame struct
 * @param   byteBuf         Pointer to the FIFO byte buffer.
 * @param   countsInByte    Number of valid bytes in @p byteBuf
 * @param   currentPos      Current parsing offset; initialize to 0 before the first call
 */
bool
ICM42688_FIFO_Parse_One_Byte_Frame(ICM42688_Handle_t *handle, ICM42688_FIFO_Frame_t *frame, const uint8_t *byteBuf,
                                   uint16_t countsInByte, uint16_t *currentPos)
{
    if (!handle || !frame || !byteBuf || !currentPos)
        return false;

    if (*currentPos >= countsInByte)
        return false;

    uint8_t                _header           = byteBuf[*currentPos];
    ICM42688_FIFO_Packet_t _fifo_packet_type = FIFO_PACKET_INVALID;
    uint8_t                _fifo_packet_size = 0U;
    const uint8_t         *_packet           = &byteBuf[*currentPos];

    bool _status = ICM42688_Get_FIFO_Packet_Info_From_Header(_header, &_fifo_packet_type, &_fifo_packet_size);
    if (!_status)
        return false;

    if ((_fifo_packet_size == 0U) || (_fifo_packet_type == FIFO_PACKET_INVALID))
        return false;

    // Reject packets that would run past the valid bytes captured from FIFO_DATA.
    if ((uint32_t)*currentPos + (uint32_t)_fifo_packet_size > (uint32_t)countsInByte)
        return false;

    _status = ICM42688_FIFO_Parse_Frame(handle, frame, _packet, _fifo_packet_size, _fifo_packet_type);
    if (!_status)
        return false;

    // Advance to the next FIFO packet in the byte buffer.
    *currentPos += (uint16_t)_fifo_packet_size;

    return true;
}



/**
 * @brief   Calibrate FIFO raw data in frame with corresponding offset and scale,
 *          and fill the calibrated data into output buffer.
 * @param   handle          Pointer to an ICM42688 Handle struct
 * @param   frame           Pointer to an ICM42688 FIFO frame struct that carries raw data to be
 *                          calibrated
 * @param   offset          Pointer to an ICM42688 Offset struct that carries the raw offset data to
 *                          be used for calibration
 * @param   outCalibratedData   Pointer to ICM42688 Scaled struct that carries the calibrated output
 *                              data
 * @note    Only fields marked valid in @p frame are calibrated; absent sensors remain zeroed.
 */
bool
ICM42688_Calibrate_FIFO_Frame(const ICM42688_Handle_t *handle, const ICM42688_FIFO_Frame_t *frame,
                              const ICM42688_Offset_Raw_t *offset, ICM42688_Temp_Accel_Gyro_Scaled_t *outCalibratedData)
{
    if (!handle || !frame || !offset || !outCalibratedData) {
        return false;
    }

    if (!frame->accel_valid && !frame->gyro_valid) {
        return false;
    }

    const float _accel_g_per_lsb  = handle->accel_g_per_lsb;
    const float _gyro_dps_per_lsb = handle->gyro_dps_per_lsb;

    memset(outCalibratedData, 0, sizeof(*outCalibratedData));

    if (frame->temp_valid) {
        outCalibratedData->temp_c = frame->gat_scaled.temp_c;
    }

    if (frame->accel_valid) {
        if (_accel_g_per_lsb <= 0.0f)
            return false;

        for (uint8_t _i = 0; _i < 3; _i++) {
            outCalibratedData->accel_g[_i] =
                frame->gat_scaled.accel_g[_i] - (offset->offset_raw_accel[_i] * _accel_g_per_lsb);
        }
    }

    if (frame->gyro_valid) {
        if (_gyro_dps_per_lsb <= 0.0f)
            return false;

        for (uint8_t _i = 0; _i < 3; _i++) {
            outCalibratedData->gyro_dps[_i] =
                frame->gat_scaled.gyro_dps[_i] - (offset->offset_raw_gyro[_i] * _gyro_dps_per_lsb);
        }
    }

    return true;
}
