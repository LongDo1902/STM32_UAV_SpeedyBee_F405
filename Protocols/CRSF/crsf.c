#include "crsf.h"
#include "crsf_protocol.h"

static volatile uint8_t    rx_byte;
static crsf_frame_packed_t rc_channel_frame;

void
crsf_init(crsf_handle_t *crsf_handle, UART_HandleTypeDef *huart)
{
    if (huart == NULL || crsf_handle == NULL) {
        // TODO: Add function to handle it
    }

    // TODO: check again with initialize value
    crsf_handle->frame_position = 0;
    crsf_handle->frame_done     = 0;
    crsf_handle->uart           = huart;
    for (int i = 0; i < CRSF_MAX_PACKET_SIZE; i++) {
        crsf_handle->crsf_frame[i] = 0;
    }
    for (int i = 0; i < CRSF_MAX_CHANNEL; i++) {
        crsf_handle->channels[i] = 0;
    }

    HAL_UART_Receive_IT(crsf_handle->uart, &rx_byte, 1);
}

void
crsf_receive_byte(crsf_handle_t *crsf_handle)
{

    if (crsf_handle == NULL) {
        return;
    }

    crsf_handle_receive_byte(crsf_handle, rx_byte);

    // Register ISR again
    HAL_UART_Receive_IT(crsf_handle->uart, &rx_byte, 1);
}

void
crsf_handle_receive_byte(crsf_handle_t *crsf_handle, uint8_t byte)
{
    if (crsf_handle == NULL) {
        return;
    }

    crsf_handle->crsf_frame[crsf_handle->frame_position++] = byte;

    if (crsf_handle->crsf_frame.frame_def.sync_byte == CRSF_SYNC_BYTE) {
        if (crsf_handle->crsf_frame.frame_def.len + 2 == crsf_handle->frame_position) {


            switch (crsf_handle->crsf_frame.frame_def.type) {
                case CRSF_FRAMETYPE_RC_CHANNELS_PACKED:
                    memcpy(&rc_channel_frame, &crsf_handle->crsf_frame,
                           sizeof(crsf_handle->crsf_frame));
                    break;

                default:
                    break;
            }

            crsf_handle->frame_position = 0;
            crsf_handle->frame_done     = 1;
        }
    }
    else {
        crsf_handle->frame_position = 0;
    }
    // TODO: Check timeout
}


bool
crsf_update(crsf_handle_t *crsf_handle)
{
    if (crsf_handle == NULL) {
        return false;
    }

    if (!crsf_handle->frame_done) {
        return false;
    }

    crsf_handle->frame_done = 0;

    // Check crc of packed
    uint8_t crc = crsf_compute_crc();
    if (crc != frame->frame_def.crc) {
        return false;
    }

    // Handle packet type
    const crsf_frame_packed_t const *frame = &rc_channel_frame;
    switch (frame->frame_def.type) {
        case CRSF_FRAMETYPE_RC_CHANNELS_PACKED:
            crsf_handle_rc_channels_packed(frame->payload, crsf_handle->channels);

            break;
        default:
            break;
    }

    return true;
}

uint16_t
crsf_get_channel(crsf_handle_t *handle, uint8_t channel)
{
    if (handle == NULL) {
        return CRSF_VALUE_CHANNEL_MID;
    }

    if (channel >= CRSF_MAX_CHANNEL || handle->signal_ok == 0) {
        return CRSF_VALUE_CHANNEL_MID;
    }

    return handle->channels[channel];
}

void
crsf_handle_rc_channels_packed(const uint8_t *payload, uint16_t *channels)
{

    const crsf_channels_packed_t const *packed = (const crsf_channels_packed_t *)payload;

    channels[0] = packed->ch0;
    channels[1] = packed->ch1;
    channels[2] = packed->ch2;
    channels[3] = packed->ch3;
    channels[4] = packed->ch4;
}

uint8_t
crsf_compute_crc(const uint8_t *data, uint8_t len)
{
    uint8_t crc = 0;

    for (uint8_t i = 0; i < len; i++) {
        crc ^= data[i];

        for (int j = 0; j < 8; j++) {
            if (crc & 0x80) {
                crc = (crc << 0x01) ^ 0xD5;
            }
            else {
                crc = crc << 1;
            }
        }
    }

    return crc;
}
