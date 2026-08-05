/*
 * icm42688_fifo.c
 *
 *  Created on: Mar 12, 2026
 *      Author: dobaolong
 */

#include "imu/features/icm42688_fifo.h"
#include "string.h"

/* ==================================================================================
 *	FIFO CONFIG
 * ================================================================================== */

bool
ICM42688_Set_FIFO_Count_Endian(ICM42688_Handle_t *pHandle, ICM42688_FIFO_Count_Endian_t countEndian)
{
    if (!pHandle)
        return false;

    if (((uint8_t)countEndian != 0U && (uint8_t)countEndian != 1U))
        return false;

    bool _status = ICM42688_Update_Reg_Bits(pHandle, ICM42688_UB0_INTF_CONF0, ICM42688_FIFO_COUNT_ENDIAN_Msk,
                                            ICM42688_FIFO_COUNT_ENDIAN_Val(countEndian));
    if (!_status)
        return false;

    pHandle->fifo_config.fifo_count_endian = countEndian;

    return true;
}



bool
ICM42688_Set_FIFO_Count_Rec(ICM42688_Handle_t *pHandle, ICM42688_FIFO_Count_Rec_t countRecord)
{
    if (!pHandle)
        return false;

    if (((uint8_t)countRecord != 0U) && ((uint8_t)countRecord != 1U))
        return false;

    bool _status = ICM42688_Update_Reg_Bits(pHandle, ICM42688_UB0_INTF_CONF0, ICM42688_FIFO_COUNT_REC_Msk,
                                            ICM42688_FIFO_COUNT_REC_Val(countRecord));
    if (!_status)
        return false;

    pHandle->fifo_config.fifo_count_rec = countRecord;

    return true;
}



bool
ICM42688_Set_FIFO_Mode(ICM42688_Handle_t *pHandle, ICM42688_FIFO_Mode_t mode)
{
    if (!pHandle)
        return false;

    if ((uint8_t)mode > (uint8_t)STOP_ON_FULL)
        return false;

    bool _status = ICM42688_Update_Reg_Bits(pHandle, ICM42688_UB0_FIFO_CONF, ICM42688_FIFO_MODE_Msk,
                                            ICM42688_FIFO_MODE_Val(mode));

    if (!_status)
        return false;

    pHandle->fifo_config.fifo_mode = (ICM42688_FIFO_Mode_t)mode;

    return true;
}



bool
ICM42688_Get_FIFO_Mode(ICM42688_Handle_t *pHandle, ICM42688_FIFO_Mode_t *pMode)
{
    if (!pHandle || !pMode)
        return false;

    uint8_t _reg    = 0U;
    bool    _status = ICM42688_ReadReg(pHandle, ICM42688_UB0_FIFO_CONF, &_reg);
    if (!_status)
        return false;

    uint8_t _raw_mode             = (uint8_t)((_reg & ICM42688_FIFO_MODE_Msk) >> ICM42688_FIFO_MODE_Pos);
    bool    _is_mode_stop_on_full = (_raw_mode == 2U) || (_raw_mode == 3U);

    *pMode = (_is_mode_stop_on_full) ? STOP_ON_FULL : (ICM42688_FIFO_Mode_t)_raw_mode;

    pHandle->fifo_config.fifo_mode = (ICM42688_FIFO_Mode_t)*pMode;

    return true;
}



bool
ICM42688_Set_FIFO_Gyro_Enable(ICM42688_Handle_t *pHandle, ICM42688_FIFO_GAT_En_t state)
{
    if (!pHandle)
        return false;

    if (((uint8_t)state != 0U) && ((uint8_t)state != 1U))
        return false;

    bool _status = ICM42688_Update_Reg_Bits(pHandle, ICM42688_UB0_FIFO_CONF1, ICM42688_FIFO_GYRO_EN_Msk,
                                            ICM42688_FIFO_GYRO_EN_Val(state));
    if (!_status)
        return false;

    pHandle->fifo_config.fifo_gyro_state = (ICM42688_FIFO_GAT_En_t)state;

    return true;
}



bool
ICM42688_Set_FIFO_Accel_Enable(ICM42688_Handle_t *pHandle, ICM42688_FIFO_GAT_En_t state)
{
    if (!pHandle)
        return false;

    if (((uint8_t)state != 0U) && ((uint8_t)state != 1U))
        return false;

    bool _status = ICM42688_Update_Reg_Bits(pHandle, ICM42688_UB0_FIFO_CONF1, ICM42688_FIFO_ACCEL_EN_Msk,
                                            ICM42688_FIFO_ACCEL_EN_Val(state));
    if (!_status)
        return false;

    pHandle->fifo_config.fifo_accel_state = (ICM42688_FIFO_GAT_En_t)state;

    return true;
}



bool
ICM42688_Set_FIFO_Temp_Enable(ICM42688_Handle_t *pHandle, ICM42688_FIFO_GAT_En_t state)
{
    if (!pHandle)
        return false;

    if (((uint8_t)state != 0U) && ((uint8_t)state != 1U))
        return false;

    bool _status = ICM42688_Update_Reg_Bits(pHandle, ICM42688_UB0_FIFO_CONF1, ICM42688_FIFO_TEMP_EN_Msk,
                                            ICM42688_FIFO_TEMP_EN_Val(state));
    if (!_status)
        return false;

    pHandle->fifo_config.fifo_temp_state = (ICM42688_FIFO_GAT_En_t)state;

    return true;
}



bool
ICM42688_Set_FIFO_HIRES_Enable(ICM42688_Handle_t *pHandle, ICM42688_FIFO_Hires_En_t state)
{
    if (!pHandle)
        return false;

    if (((uint8_t)state != 0U) && ((uint8_t)state != 1U))
        return false;

    bool _status = ICM42688_Update_Reg_Bits(pHandle, ICM42688_UB0_FIFO_CONF1, ICM42688_FIFO_HIRES_EN_Msk,
                                            ICM42688_FIFO_HIRES_EN_Val(state));
    if (!_status)
        return false;

    pHandle->fifo_config.fifo_hires_state = (ICM42688_FIFO_Hires_En_t)state;

    return true;
}



bool
ICM42688_Set_FIFO_WM_GT_THS(ICM42688_Handle_t *pHandle, ICM42688_FIFO_WM_Mode_t state)
{
    if (!pHandle)
        return false;

    if (((uint8_t)state != 0U) && ((uint8_t)state != 1U))
        return false;

    bool _status = ICM42688_Update_Reg_Bits(pHandle, ICM42688_UB0_FIFO_CONF1, ICM42688_FIFO_WM_GT_TH_Msk,
                                            ICM42688_FIFO_WM_GT_TH_Val(state));
    if (!_status)
        return false;

    pHandle->fifo_config.fifo_wm_mode = (ICM42688_FIFO_WM_Mode_t)state;

    return true;
}



bool
ICM42688_Set_FIFO_Resume_Partial_Read(ICM42688_Handle_t *pHandle, ICM42688_FIFO_Resume_Read_t state)
{
    if (!pHandle)
        return false;

    if (((uint8_t)state != 0U) && ((uint8_t)state != 1U))
        return false;

    bool _status =
        ICM42688_Update_Reg_Bits(pHandle, ICM42688_UB0_FIFO_CONF1, ICM42688_FIFO_RESUME_PARTIAL_RD_Msk,
                                 ICM42688_FIFO_RESUME_PARTIAL_RD_Val(state));
    if (!_status)
        return false;

    pHandle->fifo_config.fifo_partial_read_state = (ICM42688_FIFO_Resume_Read_t)state;

    return true;
}



bool
ICM42688_Set_FIFO_Watermark(ICM42688_Handle_t *pHandle, uint16_t fifoWatermark)
{
    if (!pHandle)
        return false;

    if ((fifoWatermark == 0U) || (fifoWatermark > 0x0FFFU))
        return false;

    // FIFO watermark is a 12-bit value: FIFO_CONF2 stores bits[7:0], FIFO_CONF3 stores bits[11:8].
    uint8_t _fifo_lower_wm = (uint8_t)(fifoWatermark & 0x00FFU);
    uint8_t _fifo_upper_wm = (uint8_t)((fifoWatermark >> 8) & 0x0FU);

    bool _status = ICM42688_Update_Reg_Bits(pHandle, ICM42688_UB0_FIFO_CONF2, ICM42688_FIFO_WM_LOWER_Msk,
                                            ICM42688_FIFO_WM_LOWER_Val(_fifo_lower_wm));
    if (!_status)
        return false;

    _status = ICM42688_Update_Reg_Bits(pHandle, ICM42688_UB0_FIFO_CONF3, ICM42688_FIFO_WM_UPPER_Msk,
                                       ICM42688_FIFO_WM_UPPER_Val(_fifo_upper_wm));
    if (!_status)
        return false;

    pHandle->fifo_config.fifo_watermark = (uint16_t)fifoWatermark;

    return true;
}



bool
ICM42688_Get_FIFO_Watermark(ICM42688_Handle_t *pHandle, uint16_t *pFifoWatermark)
{
    if (!pHandle || !pFifoWatermark)
        return false;

    uint8_t _fifo_wm_buf[2];
    bool    _status = ICM42688_ReadRegs(pHandle, ICM42688_UB0_FIFO_CONF2, _fifo_wm_buf, 2);
    if (!_status)
        return false;

    *pFifoWatermark = (uint16_t)((uint16_t)((_fifo_wm_buf[1] & ICM42688_FIFO_WM_UPPER_Msk) << 8U) |
                                 (uint16_t)((_fifo_wm_buf[0] & ICM42688_FIFO_WM_LOWER_Msk)));

    pHandle->fifo_config.fifo_watermark = (uint16_t)*pFifoWatermark;

    return true;
}



bool
ICM42688_Get_FIFO_Count(ICM42688_Handle_t *pHandle, uint16_t *pFifoCount)
{
    if (!pHandle || !pFifoCount)
        return false;

    uint8_t _fifo_count_buf[2];
    bool    _status = ICM42688_ReadRegs(pHandle, ICM42688_UB0_FIFO_COUNTH, _fifo_count_buf, 2);
    if (!_status)
        return false;

    // FIFO_COUNTH is the full high byte of FIFO_COUNT; only FIFO watermark upper bits are nibble-sized.
    if (pHandle->fifo_config.fifo_count_endian == FIFO_COUNT_BIG_ENDIAN) {
        *pFifoCount = (uint16_t)(((uint16_t)_fifo_count_buf[0] << 8) | ((uint16_t)_fifo_count_buf[1]));
    }
    else {
        *pFifoCount = (uint16_t)(((uint16_t)_fifo_count_buf[1] << 8) | ((uint16_t)_fifo_count_buf[0]));
    }

    // In byte-count mode, FIFO_COUNT cannot exceed the 2 KB FIFO depth.
    if ((pHandle->fifo_config.fifo_count_rec == FIFO_COUNT_IN_BYTE) && (*pFifoCount > 2048U)) {
        return false;
    }

    pHandle->fifo_config.fifo_count = (uint16_t)*pFifoCount;

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



bool
ICM42688_Get_FIFO_Packet_Info_From_Header(uint8_t inputHeader, ICM42688_FIFO_Packet_t *pPacketType,
                                          uint8_t *pPacketSize)
{
    if (!pPacketType || !pPacketSize)
        return false;

    const bool _has_msg   = ICM42688_FIFO_Header_Has(inputHeader, ICM42688_FIFO_HEADER_MSG_Msk);
    const bool _has_accel = ICM42688_FIFO_Header_Has(inputHeader, ICM42688_FIFO_HEADER_ACCEL_Msk);
    const bool _has_gyro  = ICM42688_FIFO_Header_Has(inputHeader, ICM42688_FIFO_HEADER_GYRO_Msk);
    const bool _has_20bit = ICM42688_FIFO_Header_Has(inputHeader, ICM42688_FIFO_HEADER_20_Msk);

    if (_has_msg) {
        *pPacketType = FIFO_PACKET_INVALID;
        *pPacketSize = 0U;
        return false;
    }

    // A FIFO data packet must contain accel, gyro, or both. Message packets are rejected above.
    if (!_has_accel && !_has_gyro) {
        *pPacketType = FIFO_PACKET_INVALID;
        *pPacketSize = 0U;
        return false;
    }

    // Packet 1: Accel only, 8 bytes
    if (_has_accel && !_has_gyro && !_has_20bit) {
        *pPacketType = FIFO_PACKET_1;
        *pPacketSize = 8U;
        return true;
    }

    // Packet 2: Gyro only, 8 bytes
    if (_has_gyro && !_has_accel && !_has_20bit) {
        *pPacketType = FIFO_PACKET_2;
        *pPacketSize = 8U;
        return true;
    }

    // Packet 3: Accel + Gyro, 16 bytes
    if (_has_accel && _has_gyro && !_has_20bit) {
        *pPacketType = FIFO_PACKET_3;
        *pPacketSize = 16U;
        return true;
    }

    // Packet 4: Accel + Gyro + high-resolution extension, 20 bytes
    if (_has_accel && _has_gyro && _has_20bit) {
        *pPacketType = FIFO_PACKET_4;
        *pPacketSize = 20U;
        return true;
    }

    // No valid info from FIFO header
    *pPacketType = FIFO_PACKET_INVALID;
    *pPacketSize = 0U;

    return false;
}



bool
ICM42688_FIFO_Parse_Frame(ICM42688_Handle_t *pHandle, ICM42688_FIFO_Frame_t *pFrame, const uint8_t *pData,
                          uint8_t packetSize, ICM42688_FIFO_Packet_t packetType)
{
    if (!pHandle || !pData || !pFrame)
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

    if ((_expected_size != packetSize) || (packetSize > sizeof(pFrame->raw)))
        return false;

    // Start from a known state so fields from a previous packet cannot leak into this frame.
    memset(pFrame, 0, sizeof(*pFrame));

    // Store packet metadata before decoding payload fields.
    pFrame->header      = pData[0];
    pFrame->packet_type = packetType;
    pFrame->packet_size = packetSize;

    pFrame->timestamp_fsync_mode = ICM42688_Get_FIFO_TimestampFsync_Mode(pFrame->header);

    pFrame->odr_accel_changed = ICM42688_FIFO_Header_Has(pFrame->header, ICM42688_FIFO_HEADER_ODR_ACCEL_Msk);

    pFrame->odr_gyro_changed = ICM42688_FIFO_Header_Has(pFrame->header, ICM42688_FIFO_HEADER_ODR_GYRO_Msk);

    // Keep a copy of the raw packet for debugging and downstream inspection.
    for (uint16_t _i = 0U; _i < packetSize; _i++) {
        pFrame->raw[_i] = pData[_i];
    }

    /* Timestamp/FSYNC mode:
     *      00: none
     *      01: reserved
     *      10: ODR timestamp
     *      11: FSYNC time */
    if (pFrame->timestamp_fsync_mode == 1U) {
        return false;
    }

    pFrame->timestamp_valid = (pFrame->timestamp_fsync_mode == 2U) || (pFrame->timestamp_fsync_mode == 3U);

    pFrame->timestamp_fsync_valid = (pFrame->timestamp_fsync_mode == 3U);

    switch (pFrame->packet_type) {
        case FIFO_PACKET_1: {
            pFrame->accel_valid = true;
            pFrame->gyro_valid  = false;
            pFrame->temp_valid  = true;
            pFrame->hires_valid = false;

            pFrame->accel_raw16[0] = ICM42688_Decode_BE16_Signed(pData[1], pData[2]);
            pFrame->accel_raw16[1] = ICM42688_Decode_BE16_Signed(pData[3], pData[4]);
            pFrame->accel_raw16[2] = ICM42688_Decode_BE16_Signed(pData[5], pData[6]);

            // Convert raw accel to g when the accel scale factor has already been configured.
            if (pHandle->accel_g_per_lsb > 0.0f) {
                float _s = pHandle->accel_g_per_lsb;

                pFrame->gat_scaled.accel_g[0] = (float)(pFrame->accel_raw16[0] * _s);
                pFrame->gat_scaled.accel_g[1] = (float)(pFrame->accel_raw16[1] * _s);
                pFrame->gat_scaled.accel_g[2] = (float)(pFrame->accel_raw16[2] * _s);
            }

            pFrame->temp_raw8         = (int8_t)pData[7];
            pFrame->gat_scaled.temp_c = ((float)pFrame->temp_raw8 / 2.07f) + 25.0f;

            break;
        }

        case FIFO_PACKET_2: {
            pFrame->accel_valid = false;
            pFrame->gyro_valid  = true;
            pFrame->temp_valid  = true;
            pFrame->hires_valid = false;

            pFrame->gyro_raw16[0] = ICM42688_Decode_BE16_Signed(pData[1], pData[2]);
            pFrame->gyro_raw16[1] = ICM42688_Decode_BE16_Signed(pData[3], pData[4]);
            pFrame->gyro_raw16[2] = ICM42688_Decode_BE16_Signed(pData[5], pData[6]);

            // Convert raw gyro to dps when the gyro scale factor has already been configured.
            if (pHandle->gyro_dps_per_lsb > 0.0f) {
                float _s = pHandle->gyro_dps_per_lsb;

                pFrame->gat_scaled.gyro_dps[0] = (float)(pFrame->gyro_raw16[0] * _s);
                pFrame->gat_scaled.gyro_dps[1] = (float)(pFrame->gyro_raw16[1] * _s);
                pFrame->gat_scaled.gyro_dps[2] = (float)(pFrame->gyro_raw16[2] * _s);
            }

            pFrame->temp_raw8         = (int8_t)pData[7];
            pFrame->gat_scaled.temp_c = ((float)pFrame->temp_raw8 / 2.07f) + 25.0f;

            break;
        }

        case FIFO_PACKET_3: {
            pFrame->accel_valid = true;
            pFrame->gyro_valid  = true;
            pFrame->temp_valid  = true;
            pFrame->hires_valid = false;

            pFrame->accel_raw16[0] = ICM42688_Decode_BE16_Signed(pData[1], pData[2]);
            pFrame->accel_raw16[1] = ICM42688_Decode_BE16_Signed(pData[3], pData[4]);
            pFrame->accel_raw16[2] = ICM42688_Decode_BE16_Signed(pData[5], pData[6]);

            pFrame->gyro_raw16[0] = ICM42688_Decode_BE16_Signed(pData[7], pData[8]);
            pFrame->gyro_raw16[1] = ICM42688_Decode_BE16_Signed(pData[9], pData[10]);
            pFrame->gyro_raw16[2] = ICM42688_Decode_BE16_Signed(pData[11], pData[12]);

            // Convert raw accel and gyro to g and dps when both scale factors are configured.
            if ((pHandle->gyro_dps_per_lsb > 0.0f) && (pHandle->accel_g_per_lsb > 0.0f)) {
                float _sa = pHandle->accel_g_per_lsb;
                float _sg = pHandle->gyro_dps_per_lsb;

                pFrame->gat_scaled.accel_g[0] = (float)(pFrame->accel_raw16[0] * _sa);
                pFrame->gat_scaled.accel_g[1] = (float)(pFrame->accel_raw16[1] * _sa);
                pFrame->gat_scaled.accel_g[2] = (float)(pFrame->accel_raw16[2] * _sa);

                pFrame->gat_scaled.gyro_dps[0] = (float)(pFrame->gyro_raw16[0] * _sg);
                pFrame->gat_scaled.gyro_dps[1] = (float)(pFrame->gyro_raw16[1] * _sg);
                pFrame->gat_scaled.gyro_dps[2] = (float)(pFrame->gyro_raw16[2] * _sg);
            }

            pFrame->temp_raw8         = (int8_t)pData[13];
            pFrame->gat_scaled.temp_c = ((float)pFrame->temp_raw8 / 2.07f) + 25.0f;

            pFrame->timestamp = ICM42688_Decode_BE16_Unsigned(pData[14], pData[15]);

            break;
        }

        case FIFO_PACKET_4: {
            pFrame->accel_valid = true;
            pFrame->gyro_valid  = true;
            pFrame->temp_valid  = true;
            pFrame->hires_valid = true;

            uint32_t _accel_x_raw =
                (uint32_t)(pData[1] << 12) | (uint32_t)(pData[2] << 4) | (uint32_t)((pData[17] >> 4) & 0x0F);

            uint32_t _accel_y_raw =
                (uint32_t)(pData[3] << 12) | (uint32_t)(pData[4] << 4) | (uint32_t)((pData[18] >> 4) & 0x0F);

            uint32_t _accel_z_raw =
                (uint32_t)(pData[5] << 12) | (uint32_t)(pData[6] << 4) | (uint32_t)((pData[19] >> 4) & 0x0F);

            uint32_t _gyro_x_raw =
                (uint32_t)(pData[7] << 12) | (uint32_t)(pData[8] << 4) | (uint32_t)(pData[17] & 0x0F);

            uint32_t _gyro_y_raw =
                (uint32_t)(pData[9] << 12) | (uint32_t)(pData[10] << 4) | (uint32_t)(pData[18] & 0x0F);

            uint32_t _gyro_z_raw =
                (uint32_t)(pData[11] << 12) | (uint32_t)(pData[12] << 4) | (uint32_t)(pData[19] & 0x0F);

            pFrame->accel_raw20[0] = ICM42688_SignExtend20(_accel_x_raw);
            pFrame->accel_raw20[1] = ICM42688_SignExtend20(_accel_y_raw);
            pFrame->accel_raw20[2] = ICM42688_SignExtend20(_accel_z_raw);

            pFrame->gyro_raw20[0] = ICM42688_SignExtend20(_gyro_x_raw);
            pFrame->gyro_raw20[1] = ICM42688_SignExtend20(_gyro_y_raw);
            pFrame->gyro_raw20[2] = ICM42688_SignExtend20(_gyro_z_raw);

            // Packet 4 high-resolution samples use fixed 20-bit scale factors.
            const float _accel_g_per_lsb_p4  = 1.0f / 8192.0f;
            const float _gyro_dps_per_lsb_p4 = 1.0f / 131.0f;

            pFrame->gat_scaled.accel_g[0] = _accel_g_per_lsb_p4 * (pFrame->accel_raw20[0]);
            pFrame->gat_scaled.accel_g[1] = _accel_g_per_lsb_p4 * (pFrame->accel_raw20[1]);
            pFrame->gat_scaled.accel_g[2] = _accel_g_per_lsb_p4 * (pFrame->accel_raw20[2]);

            pFrame->gat_scaled.gyro_dps[0] = _gyro_dps_per_lsb_p4 * (pFrame->gyro_raw20[0]);
            pFrame->gat_scaled.gyro_dps[1] = _gyro_dps_per_lsb_p4 * (pFrame->gyro_raw20[1]);
            pFrame->gat_scaled.gyro_dps[2] = _gyro_dps_per_lsb_p4 * (pFrame->gyro_raw20[2]);

            pFrame->temp_raw16        = ICM42688_Decode_BE16_Signed(pData[13], pData[14]);
            pFrame->gat_scaled.temp_c = (float)((pFrame->temp_raw16) / 132.48f) + 25.0f;

            pFrame->timestamp = ICM42688_Decode_BE16_Unsigned(pData[15], pData[16]);

            break;
        }

        default:
            return false;
    }

    return true;
}



bool
ICM42688_Get_FIFO_Frame_In_Record(ICM42688_Handle_t *pHandle, ICM42688_FIFO_Frame_t *pFrame)
{
    if (!pHandle || !pFrame)
        return false;

    // FIFO mode must not be BYPASS.
    if (pHandle->fifo_config.fifo_mode == BYPASS)
        return false;

    // Record mode reports how many complete FIFO packets are available.
    if (pHandle->fifo_config.fifo_count_rec != FIFO_COUNT_IN_RECORD)
        return false;

    // Partial-read resume is required because this path reads the header first, then the remaining packet
    // bytes.
    if (pHandle->fifo_config.fifo_partial_read_state != FIFO_PARTIAL_READ_ENABLE)
        return false;

    // Get the total available number of packets/records.
    uint16_t _fifo_count_in_record = 0U;

    bool _status = ICM42688_Get_FIFO_Count(pHandle, &_fifo_count_in_record);
    if (!_status)
        return false;

    if (_fifo_count_in_record < 1U)
        return false;

    // Read the packet header first so packet type and size can be decoded.
    uint8_t                _header           = 0U;
    ICM42688_FIFO_Packet_t _fifo_packet_type = FIFO_PACKET_INVALID;
    uint8_t                _fifo_packet_size = 0U;

    // FIFO_DATA returns the next unread byte, which is the packet header in record mode.
    _status = ICM42688_ReadRegs(pHandle, ICM42688_UB0_FIFO_DATA, &_header, 1);
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
    _status                = ICM42688_ReadRegs(pHandle, ICM42688_UB0_FIFO_DATA, &_fifo_data[1],
                                               (uint16_t)(_fifo_packet_size - 1U));
    if (!_status)
        return false;

    // Decode the packet into the output frame.
    _status = ICM42688_FIFO_Parse_Frame(pHandle, pFrame, _fifo_data, _fifo_packet_size, _fifo_packet_type);
    if (!_status)
        return false;

    return true;
}



bool
ICM42688_Get_FIFO_Frame_In_Byte(ICM42688_Handle_t *pHandle, uint8_t *pRawBuf, uint16_t rawSize)
{
    if (!pHandle || !pRawBuf || rawSize == 0U)
        return false;

    // FIFO mode must not be BYPASS.
    if (pHandle->fifo_config.fifo_mode == BYPASS)
        return false;

    // Byte-count mode reports the exact number of FIFO_DATA bytes to burst read.
    if (pHandle->fifo_config.fifo_count_rec != FIFO_COUNT_IN_BYTE)
        return false;

    // Partial-read resume must be disabled because this path drains the FIFO in one burst.
    if (pHandle->fifo_config.fifo_partial_read_state != FIFO_PARTIAL_READ_DISABLE)
        return false;

    uint16_t _fifo_count_in_byte = 0U;

    bool _status = ICM42688_Get_FIFO_Count(pHandle, &_fifo_count_in_byte);
    if (!_status)
        return false;

    if ((_fifo_count_in_byte == 0U) || (_fifo_count_in_byte > rawSize))
        return false;

    _status = ICM42688_ReadRegs(pHandle, ICM42688_UB0_FIFO_DATA, pRawBuf, _fifo_count_in_byte);
    if (!_status)
        return false;

    return true;
}



bool
ICM42688_FIFO_Flush(ICM42688_Handle_t *pHandle, bool enable)
{
    if (!pHandle)
        return false;

    uint8_t _enable_val = enable ? 1U : 0U;
    bool    _status = ICM42688_Update_Reg_Bits(pHandle, ICM42688_UB0_SIGNAL_PATH_RST, ICM42688_FIFO_FLUSH_Msk,
                                               ICM42688_FIFO_FLUSH_Val(_enable_val));

    if (!_status)
        return false;

    return true;
}



bool
ICM42688_FIFO_Parse_One_Byte_Frame(ICM42688_Handle_t *pHandle, ICM42688_FIFO_Frame_t *pFrame,
                                   const uint8_t *pByteBuf, uint16_t countsInByte, uint16_t *pCurrentPos)
{
    if (!pHandle || !pFrame || !pByteBuf || !pCurrentPos)
        return false;

    if (*pCurrentPos >= countsInByte)
        return false;

    uint8_t                _header           = pByteBuf[*pCurrentPos]; // Packet header extraction
    ICM42688_FIFO_Packet_t _fifo_packet_type = FIFO_PACKET_INVALID;
    uint8_t                _fifo_packet_size = 0U;
    const uint8_t *_packet_reference_data    = &pByteBuf[*pCurrentPos]; // Pointer to a real DMA RX buffer

    bool _status = ICM42688_Get_FIFO_Packet_Info_From_Header(_header, &_fifo_packet_type, &_fifo_packet_size);
    if (!_status)
        return false;

    if ((_fifo_packet_size == 0U) || (_fifo_packet_type == FIFO_PACKET_INVALID))
        return false;

    // Reject packets that would run past the valid bytes captured from FIFO_DATA.
    if ((uint32_t)*pCurrentPos + (uint32_t)_fifo_packet_size > (uint32_t)countsInByte)
        return false;

    _status = ICM42688_FIFO_Parse_Frame(pHandle, pFrame, _packet_reference_data, _fifo_packet_size,
                                        _fifo_packet_type);
    if (!_status)
        return false;

    // Advance to the next FIFO packet in the byte buffer.
    *pCurrentPos += (uint16_t)_fifo_packet_size;

    return true;
}



bool
ICM42688_Calibrate_FIFO_Frame(const ICM42688_Handle_t *pHandle, const ICM42688_FIFO_Frame_t *pFrame,
                              const ICM42688_Offset_Raw_t       *pOffset,
                              ICM42688_Temp_Accel_Gyro_Scaled_t *pOutCalibratedData)
{
    if (!pHandle || !pFrame || !pOffset || !pOutCalibratedData) {
        return false;
    }

    if (!pFrame->accel_valid && !pFrame->gyro_valid) {
        return false;
    }

    const float _accel_g_per_lsb  = pHandle->accel_g_per_lsb;
    const float _gyro_dps_per_lsb = pHandle->gyro_dps_per_lsb;

    memset(pOutCalibratedData, 0, sizeof(*pOutCalibratedData));

    if (pFrame->temp_valid) {
        pOutCalibratedData->temp_c = pFrame->gat_scaled.temp_c;
    }

    if (pFrame->accel_valid) {
        if (_accel_g_per_lsb <= 0.0f)
            return false;

        for (uint8_t _i = 0; _i < 3; _i++) {
            pOutCalibratedData->accel_g[_i] =
                pFrame->gat_scaled.accel_g[_i] - (pOffset->offset_raw_accel[_i] * _accel_g_per_lsb);
        }
    }

    if (pFrame->gyro_valid) {
        if (_gyro_dps_per_lsb <= 0.0f)
            return false;

        for (uint8_t _i = 0; _i < 3; _i++) {
            pOutCalibratedData->gyro_dps[_i] =
                pFrame->gat_scaled.gyro_dps[_i] - (pOffset->offset_raw_gyro[_i] * _gyro_dps_per_lsb);
        }
    }

    return true;
}
