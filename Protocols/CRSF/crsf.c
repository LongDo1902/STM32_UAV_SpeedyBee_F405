#include "crsf.h"
#include "crsf_protocol.h"

static volatile uint8_t             rx_byte;
static volatile crsf_frame_packed_t rc_channel_frame;

void
crsf_init(crsf_handle_t *crsf_handle, UART_HandleTypeDef *huart)
{
    if (huart == NULL || crsf_handle == NULL) {
        return;
    }

    crsf_handle->frame_position  = 0;
    crsf_handle->crsf_frame_done = 0;
    crsf_handle->uart            = huart;

    HAL_UART_Receive_IT(crsf_handle->uart, (uint8_t *)&rx_byte, 1);
}

void
crsf_receive_byte(crsf_handle_t *crsf_handle)
{

    if (crsf_handle == NULL) {
        return;
    }

    crsf_handle_receive_byte(crsf_handle, rx_byte);

    /* Register ISR again */
    (void)crsf_start_receive(crsf_handle);
}

bool
crsf_start_receive(crsf_handle_t *crsf_handle)
{
    if (crsf_handle == NULL || crsf_handle->uart == NULL) {
        return false;
    }

    return HAL_UART_Receive_IT(crsf_handle->uart, (uint8_t *)&rx_byte, 1) == HAL_OK;
}


bool
crsf_update(crsf_handle_t *crsf_handle)
{
    if (crsf_handle->uart == NULL) {
        return false;
    }

    if (!crsf_handle->crsf_frame_done) {
        return false;
    }

    /* Reset frame done flag */
    crsf_handle->crsf_frame_done = false;

    /* Unpack frame */
    switch (rc_channel_frame.frame_def.type) {
        case CRSF_FRAMETYPE_RC_CHANNELS_PACKED:
            crsf_handle_rc_channels_packed(rc_channel_frame.frame_def.payload,
                                           crsf_handle->channels);
            break;

        default:
            break;
    }

    return true;
}

void
crsf_handle_rc_channels_packed(const uint8_t *payload, uint16_t *channels)
{
    const crsf_rc_channels_packed_t const *packed = (const crsf_rc_channels_packed_t *)payload;

    channels[0] = packed->ch1;
    channels[1] = packed->ch2;
    channels[2] = packed->ch3;
    channels[3] = packed->ch4;
    channels[4] = packed->ch5;

    /* Scale for rc channels
     *       RC     PWM
     * min  172 ->  988us
     * mid  992 -> 1500us
     * max 1811 -> 2012us
     */
    for (uint8_t i = 0; i < CRSF_MAX_CHANNEL; i++) {
        channels[i] = (CRSF_RC_CHANNEL_SCALE_LEGACY * channels[i]) + 881;
    }
}

void
crsf_handle_receive_byte(crsf_handle_t *crsf_handle, uint8_t byte)
{
    if (crsf_handle == NULL) {
        return;
    }


    crsf_handle->crsf_frame.bytes[crsf_handle->frame_position] = byte;
    if (crsf_handle->crsf_frame.frame_def.sync_byte == CRSF_SYNC_BYTE) {
        crsf_handle->frame_position++;
    }

    /* RC frame */
    const uint8_t full_frame_length = CRSF_MAX_PAYLOAD_RC_CHANNELS + 4;
    if (crsf_handle->frame_position < full_frame_length) {
        return;
    }

    crsf_handle->frame_position = 0;

    /* Check crc of packed */
    uint8_t crc = crsf_compute_crc((const uint8_t *)&crsf_handle->crsf_frame.frame_def.type,
                                   crsf_handle->crsf_frame.frame_def.len - 1);
    if (crc != crsf_handle->crsf_frame.frame_def.crc) {
        return;
    }

    crsf_handle->crsf_frame_done = 1;

    switch (crsf_handle->crsf_frame.frame_def.type) {
        case CRSF_FRAMETYPE_RC_CHANNELS_PACKED:
            // Copy to avoid race condition
            memcpy(&rc_channel_frame, &crsf_handle->crsf_frame, sizeof(crsf_handle->crsf_frame));
            break;

        default:
            break;
    }


    // TODO: Check timeout
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

bool
crsf_get_channel(crsf_handle_t *handle, crsf_channel_t channel, uint16_t *value)
{
    if (handle == NULL || value == NULL) {
        return false;
    }

    /* Check bound channel */
    if (channel > CRSF_MAX_CHANNEL || channel < CRSF_CHANNEL_ROLL) {
        return false;
    }

    *value = handle->channels[channel - 1];

    return true;
}

bool
crsf_get_channel_roll(crsf_handle_t *handle, uint16_t *value)
{
    return crsf_get_channel(handle, CRSF_CHANNEL_ROLL, value);
}

bool
crsf_get_channel_pitch(crsf_handle_t *handle, uint16_t *value)
{
    return crsf_get_channel(handle, CRSF_CHANNEL_PITCH, value);
}

bool
crsf_get_channel_throttle(crsf_handle_t *handle, uint16_t *value)
{
    return crsf_get_channel(handle, CRSF_CHANNEL_THROTTLE, value);
}

bool
crsf_get_channel_yaw(crsf_handle_t *handle, uint16_t *value)
{
    return crsf_get_channel(handle, CRSF_CHANNEL_YAW, value);
}

bool
crsf_get_channel_aux1(crsf_handle_t *handle, uint16_t *value)
{
    return crsf_get_channel(handle, CRSF_CHANNEL_AUX1, value);
}
