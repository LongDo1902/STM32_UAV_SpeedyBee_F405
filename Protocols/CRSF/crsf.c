// init UART + DMA
// update dữ liệu
// cung cấp channel cho hệ thống

#include "crsf.h"
#include "crsf_parser.h"
#include "crsf_protocol.h"

static volatile uint8_t rx_bytes;

void
crsf_init(crsf_handle_t *crsf_handle, UART_HandleTypeDef *huart)
{
    if (huart == NULL || crsf_handle == NULL) {
        // TODO: Add function to handle it
    }

    crsf_handle->uart = huart;

    crsf_parser_init(&crsf_handle->parser);
    HAL_UART_Receive_IT(crsf_handle->uart, &rx_bytes, 1);
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
crsf_receive_byte(crsf_handle_t *crsf_handle)
{
    crsf_parse_byte(&crsf_handle->parser, rx_bytes);

    HAL_UART_Receive_IT(crsf_handle->uart, &rx_bytes, 1);
}

bool
crsf_update(crsf_handle_t *crsf_handle)
{
    if (crsf_handle == NULL) {
        return false;
    }

    if (!crsf_handle->parser.frame_done) {
        // TODO: Add time out
        return false;
    }

    crsf_handle->parser.frame_done = 0;

    crsf_frame_t *frame = &crsf_handle->parser.frame;

    switch (frame->type) {
        case CRSF_FRAMETYPE_RC_CHANNELS_PACKED:
            crsf_handle_rc_channels_packed(frame->payload, crsf_handle->channels);

            break;
        default:
            break;
    }

    return true;
}
