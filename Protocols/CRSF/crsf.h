#ifndef _CRSF_H_
#define _CRSF_H_

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "stm32f4xx_hal.h"

#include "crsf_protocol.h"

/**
 * @brief Initialize the CRSF driver.
 *
 * @param crsf_handle Pointer to CRSF driver instance.
 * @param huart UART peripheral used for CRSF communication.
 */
void crsf_init(crsf_handle_t *handle, UART_HandleTypeDef *huart);

/**
 * @brief Arm UART interrupt reception for the next CRSF byte.
 *
 * @param handle Pointer to CRSF driver instance.
 *
 * @retval true  UART reception was started.
 * @retval false UART reception could not be started.
 */
bool crsf_start_receive(crsf_handle_t *handle);

/**
 * @brief Process a received UART byte.
 *
 * @param crsf_handle Pointer to the CRSF driver instance.
 */
void crsf_receive_byte(crsf_handle_t *crsf_handle);

/**
 * @brief  Update the CRSF driver state, processing rc frames if available.
 *
 * @param crsf_handle Pointer to the CRSF driver instance.
 *
 * @retval true  A valid RC frame was processed.
 * @retval false No complete frame is available.
 */
bool crsf_update(crsf_handle_t *handle);

/**
 * @brief Decode packed RC channel data.
 *
 * Unpacks a CRSF RC Channels Packed payload into an array of channel values.
 *
 * @param payload Pointer to the 22-byte CRSF payload.
 * @param channels Output buffer for decoded channel values.
 */
void crsf_handle_rc_channels_packed(const uint8_t *payload, uint16_t *channels);

/**
 * @brief Parse a received CRSF byte.
 *
 * @param crsf_handle Pointer to the CRSF driver instance.
 * @param byte Received UART byte.
 */
void crsf_handle_receive_byte(crsf_handle_t *crsf_handle, uint8_t byte);

/**
 * @brief Compute the CRC-8 checksum used by the CRSF protocol.
 *
 * @param data Pointer to the input data.
 * @param len Number of bytes to process.
 *
 * @return Computed CRC value.
 */
uint8_t crsf_compute_crc(const uint8_t *data, uint8_t len);

/****************************************************************
 * Helper functions
 * **************************************************************/

bool crsf_get_channel(crsf_handle_t *handle, crsf_channel_t channel, uint16_t *value);
bool crsf_get_channel_roll(crsf_handle_t *handle, uint16_t *value);
bool crsf_get_channel_pitch(crsf_handle_t *handle, uint16_t *value);
bool crsf_get_channel_yaw(crsf_handle_t *handle, uint16_t *value);
bool crsf_get_channel_throttle(crsf_handle_t *handle, uint16_t *value);
bool crsf_get_channel_aux(crsf_handle_t *handle, uint16_t *value);

#endif // _CRSF_H_
