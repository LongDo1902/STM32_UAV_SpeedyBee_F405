#ifndef _CRSF_H_
#define _CRSF_H_

#include <stdint.h>

#include "stm32f4xx_hal.h"

#include "crsf_parser.h"
#include "crsf_protocol.h"

typedef struct
{
    UART_HandleTypeDef *uart;
    crsf_parser_t       parser;
    uint16_t            channels[CRSF_MAX_CHANNEL];
    uint8_t             signal_ok;
} crsf_handle_t;

void     crsf_init(crsf_handle_t *handle, UART_HandleTypeDef *huart);
bool     crsf_update(crsf_handle_t *handle);
uint16_t crsf_get_channel(crsf_handle_t *handle, uint8_t channel);

void crsf_receive_byte(crsf_handle_t *crsf_handle, uint8_t byte);
void crsf_handle_receive_byte(crsf_handle_t *crsf_handle);

void crsf_handle_rc_channels_packed(const uint8_t *payload, uint16_t *channels);

#endif // _CRSF_H_
