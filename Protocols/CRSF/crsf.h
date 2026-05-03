#ifndef _CRSF_H_
#define _CRSF_H_

#include <stdint.h>

#include "stm32f4xx_hal.h"

#include "crsf_protocol.h"

void crsf_init(crsf_handle_t *handle, UART_HandleTypeDef *huart);
bool crsf_update(crsf_handle_t *handle);

uint16_t crsf_get_channel(crsf_handle_t *handle, uint8_t channel);

void crsf_receive_byte(crsf_handle_t *crsf_handle);
void crsf_handle_receive_byte(crsf_handle_t *crsf_handle, uint8_t byte);

void crsf_handle_rc_channels_packed(const uint8_t *payload, uint16_t *channels);

uint8_t crsf_compute_crc(const uint8_t *data, uint8_t len);

#endif // _CRSF_H_
