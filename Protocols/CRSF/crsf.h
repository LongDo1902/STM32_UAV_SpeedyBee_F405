#ifndef _CRSF_H_
#define _CRSF_H_

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "stm32f4xx_hal.h"

#include "crsf_protocol.h"

void crsf_init(crsf_handle_t *handle, UART_HandleTypeDef *huart);
void crsf_receive_byte(crsf_handle_t *crsf_handle);
bool crsf_update(crsf_handle_t *handle);

void crsf_handle_receive_byte(crsf_handle_t *crsf_handle, uint8_t byte);
void crsf_handle_rc_channels_packed(const uint8_t *payload, uint16_t *channels);

bool crsf_get_channel(crsf_handle_t *handle, crsf_channel_t channel, uint16_t *value);
bool crsf_get_channel_roll(crsf_handle_t *handle, uint16_t *value);
bool crsf_get_channel_pitch(crsf_handle_t *handle, uint16_t *value);
bool crsf_get_channel_throttle(crsf_handle_t *handle, uint16_t *value);
bool crsf_get_channel_yaw(crsf_handle_t *handle, uint16_t *value);
bool crsf_get_channel_aux(crsf_handle_t *handle, uint16_t *value);

uint8_t crsf_compute_crc(const uint8_t *data, uint8_t len);

#endif // _CRSF_H_
