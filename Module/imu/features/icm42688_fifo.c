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

    bool _status =
        ICM42688_Update_Reg_Bits(handle, ICM42688_UB0_INTF_CONF0, ICM42688_FIFO_COUNT_ENDIAN_Msk,
                                 ICM42688_FIFO_COUNT_ENDIAN_Val(countEndian));
    if (!_status)
        return false;

    handle->fifo_config.fifo_count_endian = countEndian;

    return true;
}



/**
 * @brief   Set a desired count record type
 * @param   handle      Pointer to ICM42688 Handle struct
 * @param   countRecord Number of receiving data is considered count in byte/record
 */
bool
ICM42688_Set_FIFO_Count_Rec(ICM42688_Handle_t *handle, ICM42688_FIFO_Count_Rec_t countRecord)
{
    if (!handle)
        return false;

    bool _status =
        ICM42688_Update_Reg_Bits(handle, ICM42688_UB0_INTF_CONF0, ICM42688_FIFO_COUNT_REC_Msk,
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

    bool _status = ICM42688_Update_Reg_Bits(handle, ICM42688_UB0_FIFO_CONF, ICM42688_FIFO_MODE_Msk,
                                            ICM42688_FIFO_MODE_Val(mode));

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

    uint8_t _raw_mode = (uint8_t)((_reg & ICM42688_FIFO_MODE_Msk) >> ICM42688_FIFO_MODE_Pos);
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

    bool _status =
        ICM42688_Update_Reg_Bits(handle, ICM42688_UB0_FIFO_CONF1, ICM42688_FIFO_GYRO_EN_Msk,
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

    bool _status =
        ICM42688_Update_Reg_Bits(handle, ICM42688_UB0_FIFO_CONF1, ICM42688_FIFO_ACCEL_EN_Msk,
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

    bool _status =
        ICM42688_Update_Reg_Bits(handle, ICM42688_UB0_FIFO_CONF1, ICM42688_FIFO_TEMP_EN_Msk,
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

    bool _status =
        ICM42688_Update_Reg_Bits(handle, ICM42688_UB0_FIFO_CONF1, ICM42688_FIFO_HIRES_EN_Msk,
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

    bool _status =
        ICM42688_Update_Reg_Bits(handle, ICM42688_UB0_FIFO_CONF1, ICM42688_FIFO_WM_GT_TH_Msk,
                                 ICM42688_FIFO_WM_GT_TH_Val(state));
    if (!_status)
        return false;

    handle->fifo_config.fifo_wm_mode = (ICM42688_FIFO_WM_Mode_t)state;

    return true;
}



/**
 * @brief   Set "FIFO Resume Partial Read" mode
 * @param   handle  Pointer to ICM42688 Handle struct
 * @param   state   Whole register re-read or partial register read resume
 */
bool
ICM42688_Set_FIFO_Resume_Partial_Read(ICM42688_Handle_t *handle, ICM42688_FIFO_Resume_Read_t state)
{
    if (!handle)
        return false;
    bool _status = ICM42688_Update_Reg_Bits(handle, ICM42688_UB0_FIFO_CONF1,
                                            ICM42688_FIFO_RESUME_PARTIAL_RD_Msk,
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

    uint8_t _fifo_lower_wm = (uint8_t)(fifoWatermark & 0x00FFU);
    uint8_t _fifo_upper_wm = (uint8_t)((fifoWatermark >> 8) & 0x0FU);

    bool _status =
        ICM42688_Update_Reg_Bits(handle, ICM42688_UB0_FIFO_CONF2, ICM42688_FIFO_WM_LOWER_Msk,
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
 * @brief   Read/Get the current watermark level
 * @param   handle          Pointer to ICM42688 Handle struct
 * @param   fifoWatermark   Pointer to a variable that stores the actual returned WM level
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
 * @brief   Read the actual return number of bytes/records from the sensor
 * @param   handle      Pointer to ICM42688 Handle struct
 * @param   fifoCount   Pointer to a variable that stores the actual return number of bytes/records
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

    if (handle->fifo_config.fifo_count_endian == FIFO_COUNT_BIG_ENDIAN) { // Big endian format
        *fifoCount = (uint16_t)(((uint16_t)(_fifo_count_buf[0] & 0xFFU) << 8) |
                                ((uint16_t)(_fifo_count_buf[1] & 0xFFU)));
    }
    else {
        *fifoCount = (uint16_t)(((uint16_t)(_fifo_count_buf[1] & 0xFFU) << 8) |
                                ((uint16_t)(_fifo_count_buf[0] & 0xFFU)));
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
uint8_t packet_size_ = 0U;

bool
ICM42688_Get_FIFO_Packet_Info_From_Header(uint8_t inputHeader, ICM42688_FIFO_Packet_t *packetType,
                                          uint8_t *packetSize)
{
    if (!packetType || !packetSize)
        return false;

    const bool _has_msg   = ICM42688_FIFO_Header_Has(inputHeader, ICM42688_FIFO_HEADER_MSG_Msk);
    const bool _has_accel = ICM42688_FIFO_Header_Has(inputHeader, ICM42688_FIFO_HEADER_ACCEL_Msk);
    const bool _has_gyro  = ICM42688_FIFO_Header_Has(inputHeader, ICM42688_FIFO_HEADER_GYRO_Msk);
    const bool _has_20bit = ICM42688_FIFO_Header_Has(inputHeader, ICM42688_FIFO_HEADER_20_Msk);

    if (_has_msg) {
        *packetType  = FIFO_PACKET_INVALID;
        *packetSize  = 0U;
        packet_size_ = *packetSize;
        return false;
    }

    // At lease HEADER_ACCEL and HEADER GYRO must be set to be valid
    if (!_has_accel && !_has_gyro) {
        *packetType  = FIFO_PACKET_INVALID;
        *packetSize  = 0U;
        packet_size_ = *packetSize;
        return false;
    }

    // Packet 1: Accel only, 8 bytes
    if (_has_accel && !_has_gyro && !_has_20bit) {
        *packetType  = FIFO_PACKET_1;
        *packetSize  = 8U;
        packet_size_ = *packetSize;
        return true;
    }

    // Packet 2: Gyro only, 8 bytes
    if (_has_gyro && !_has_accel && !_has_20bit) {
        *packetType  = FIFO_PACKET_2;
        *packetSize  = 8U;
        packet_size_ = *packetSize;
        return true;
    }

    // Packet 3: Accel + Gyro 16 bytes
    if (_has_accel && _has_gyro && !_has_20bit) {
        *packetType  = FIFO_PACKET_3;
        *packetSize  = 16U;
        packet_size_ = *packetSize;
        return true;
    }

    // Packet 4: Accel + Gyro + 20 bytes
    if (_has_accel && _has_gyro && _has_20bit) {
        *packetType  = FIFO_PACKET_4;
        *packetSize  = 20U;
        packet_size_ = *packetSize;
        return true;
    }

    // No valid info from FIFO header
    *packetType  = FIFO_PACKET_INVALID;
    *packetSize  = 0U;
    packet_size_ = 0U;

    return false;
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
bool
ICM42688_FIFO_Parse_Frame(ICM42688_Handle_t *handle, ICM42688_FIFO_Frame_t *frame,
                          const uint8_t *data, uint8_t packetSize,
                          ICM42688_FIFO_Packet_t packetType)
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
        return false;
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

            // Convert raw gyro to gyro in dps
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

            // Convert raw accel and gyro to accel(g) and gyro(dps)
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

            uint32_t _accel_x_raw = (uint32_t)(data[1] << 12) | (uint32_t)(data[2] << 4) |
                                    (uint32_t)((data[17] >> 4) & 0x0F);

            uint32_t _accel_y_raw = (uint32_t)(data[3] << 12) | (uint32_t)(data[4] << 4) |
                                    (uint32_t)((data[18] >> 4) & 0x0F);

            uint32_t _accel_z_raw = (uint32_t)(data[5] << 12) | (uint32_t)(data[6] << 4) |
                                    (uint32_t)((data[19] >> 4) & 0x0F);

            uint32_t _gyro_x_raw =
                (uint32_t)(data[7] << 12) | (uint32_t)(data[8] << 4) | (uint32_t)(data[17] & 0x0F);

            uint32_t _gyro_y_raw =
                (uint32_t)(data[9] << 12) | (uint32_t)(data[10] << 4) | (uint32_t)(data[18] & 0x0F);

            uint32_t _gyro_z_raw = (uint32_t)(data[11] << 12) | (uint32_t)(data[12] << 4) |
                                   (uint32_t)(data[19] & 0x0F);

            frame->accel_raw20[0] = ICM42688_SignExtend20(_accel_x_raw);
            frame->accel_raw20[1] = ICM42688_SignExtend20(_accel_y_raw);
            frame->accel_raw20[2] = ICM42688_SignExtend20(_accel_z_raw);

            frame->gyro_raw20[0] = ICM42688_SignExtend20(_gyro_x_raw);
            frame->gyro_raw20[1] = ICM42688_SignExtend20(_gyro_y_raw);
            frame->gyro_raw20[2] = ICM42688_SignExtend20(_gyro_z_raw);

            // Convert raw accel and gyro to accel(g) and gyro(dps)
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
 * @brief   Read FIFO frame when FIFO Count = Record
 * @param   handle  Pointer to ICM42688 Handle struct
 * @param   frame   Pointer to an output ICM42688 FIFO frame struct
 */
bool
ICM42688_Get_FIFO_Frame_In_Record(ICM42688_Handle_t *handle, ICM42688_FIFO_Frame_t *frame)
{
    if (!handle || !frame)
        return false;

    // FIFO mode must not be BYPASS
    if (handle->fifo_config.fifo_mode == BYPASS)
        return false;

    // Must be count in record
    if (handle->fifo_config.fifo_count_rec != FIFO_COUNT_IN_RECORD)
        return false;

    // Must enable resume partial FIFO read
    if (handle->fifo_config.fifo_partial_read_state != FIFO_PARTIAL_READ_ENABLE)
        return false;

    // Get total available number of packets/records
    uint16_t _fifo_count_in_record = 0U;

    bool _status = ICM42688_Get_FIFO_Count(handle, &_fifo_count_in_record);
    if (!_status)
        return false;

    if (_fifo_count_in_record < 1U)
        return false;

    // Extract Packet Type & Packet Size from header by reading the first byte from register
    uint8_t                _header           = 0U;
    ICM42688_FIFO_Packet_t _fifo_packet_type = FIFO_PACKET_INVALID;
    uint8_t                _fifo_packet_size = 0U;

    // Append the first 8-bits field from FIFO_DATA to @p header
    _status = ICM42688_ReadRegs(handle, ICM42688_UB0_FIFO_DATA, &_header, 1);
    if (!_status)
        return false;

    _status =
        ICM42688_Get_FIFO_Packet_Info_From_Header(_header, &_fifo_packet_type, &_fifo_packet_size);
    if (!_status)
        return false;

    if ((_fifo_packet_size == 0U) || (_fifo_packet_type == FIFO_PACKET_INVALID))
        return false;

    // Resume reading the remaining bytes in that packet
    uint8_t _fifo_data[20] = {0};
    _fifo_data[0]          = _header;
    _status                = ICM42688_ReadRegs(handle, ICM42688_UB0_FIFO_DATA, &_fifo_data[1],
                                               (uint16_t)(_fifo_packet_size - 1U));
    if (!_status)
        return false;

    // Start parsing frame
    _status =
        ICM42688_FIFO_Parse_Frame(handle, frame, _fifo_data, _fifo_packet_size, _fifo_packet_type);
    if (!_status)
        return false;

    return true;
}



/**
 * @brief   Read all availale FIFO bytes in one SPI burst
 * @note    This function is valid only when:
 *          FIFO is enabled, NOT BYPASS
 *          FIFO count is configured to report in BYTEs
 *          PARTIAL FIFO READ is DISABLED
 * In this mode, FIFO_COUNT gives the total number of bytes to read,
 * and the driver performs one burst read from FIFO_DATA
 * @param   rawBuf
 * @param   rawSize
 */
bool
ICM42688_Get_FIFO_Frame_In_Byte(ICM42688_Handle_t *handle, uint8_t *rawBuf, uint16_t rawSize)
{
    if (!handle || !rawBuf || rawSize == 0U)
        return false;

    // FIFO mode must not be BYPASS
    if (handle->fifo_config.fifo_mode == BYPASS)
        return false;

    // Use FIFO_COUNT_IN_BYTE to know how many bytes for SPI burst read to handle
    if (handle->fifo_config.fifo_count_rec != FIFO_COUNT_IN_BYTE)
        return false;

    // Must re-read the entire FIFO packet
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
 * @brief   Parse one byte of FIFO frame
 * @note    MUST call @protocol ICM42688_Get_FIFO_Frame_In_Byte before using this function
 * @param   handle          Pointer to an ICM42688 Handle struct
 * @param   frame           Pointer to an output ICM42688 FIFO frame struct
 * @param   byteBuf         Pointer to an output FIFO buffer.
 * @param   countsInByte    The counts in byte in the FIFO storage
 * @param   currentPos
 *                          @note This variable must be initialized with 0 before running this
 *                          function!
 */
bool
ICM42688_FIFO_Parse_One_Byte_Frame(ICM42688_Handle_t *handle, ICM42688_FIFO_Frame_t *frame,
                                   const uint8_t *byteBuf, uint16_t countsInByte,
                                   uint16_t *currentPos)
{
    if (!handle || !frame || !byteBuf || !currentPos)
        return false;

    if (*currentPos >= countsInByte)
        return false;

    uint8_t                _header           = byteBuf[*currentPos];
    ICM42688_FIFO_Packet_t _fifo_packet_type = FIFO_PACKET_INVALID;
    uint8_t                _fifo_packet_size = 0U;
    const uint8_t         *_packet           = &byteBuf[*currentPos];

    bool _status =
        ICM42688_Get_FIFO_Packet_Info_From_Header(_header, &_fifo_packet_type, &_fifo_packet_size);
    if (!_status)
        return false;

    if ((_fifo_packet_size == 0U) || (_fifo_packet_type == FIFO_PACKET_INVALID))
        return false;

    // Avoid overflow the real number of counts in byte
    if ((uint32_t)*currentPos + (uint32_t)_fifo_packet_size > (uint32_t)countsInByte)
        return false;

    _status =
        ICM42688_FIFO_Parse_Frame(handle, frame, _packet, _fifo_packet_size, _fifo_packet_type);
    if (!_status)
        return false;

    // Move to the next/different FIFO packet
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
 */
bool
ICM42688_Calibrate_FIFO_Frame(const ICM42688_Handle_t *handle, const ICM42688_FIFO_Frame_t *frame,
                              const ICM42688_Offset_Raw_t       *offset,
                              ICM42688_Temp_Accel_Gyro_Scaled_t *outCalibratedData)
{
    if (!handle || !frame || !offset || !outCalibratedData) {
        return false;
    }

    if (!frame->accel_valid && !frame->gyro_valid) {
        return false;
    }

    const float _accel_g_per_lsb  = handle->accel_g_per_lsb;
    const float _gyro_dps_per_lsb = handle->gyro_dps_per_lsb;

    for (uint8_t i = 0; i < 3; i++) {
        outCalibratedData->accel_g[i] =
            frame->gat_scaled.accel_g[i] - (offset->offset_raw_accel[i] * _accel_g_per_lsb);
    }

    for (uint8_t i = 0; i < 3; i++) {
        outCalibratedData->gyro_dps[i] =
            frame->gat_scaled.gyro_dps[i] - (offset->offset_raw_gyro[i] * _gyro_dps_per_lsb);
    }

    return true;
}