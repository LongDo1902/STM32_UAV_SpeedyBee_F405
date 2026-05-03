/*
 * REF: https://github.com/crsf-wg/crsf/wiki/Message-Format
 * Format: [address] [len] [type] [payload] [crc8]
 *
 * */

#ifndef _CRSF_PROTOCOL_H_
#define _CRSF_PROTOCOL_H_

#include <stdint.h>

#define CRSF_MAX_CHANNEL 5 // total 16 channels but only support 5 channels
#define CRSF_BITS_PER_CHANNEL 11
#define CRSF_MAX_PAYLOAD_SIZE 60
#define CRSF_MAX_PACKET_SIZE (CRSF_MAX_PAYLOAD_SIZE + 4)

#define CRSF_SYNC_BYTE (0xC8)                    // Address of Flight Controller
#define CRSF_FRAMETYPE_RC_CHANNELS_PACKED (0x16) // Frame types of rc channels

#define CRSF_VALUE_CHANNEL_MIN 172
#define CRSF_VALUE_CHANNEL_MID 992
#define CRSF_VALUE_CHANNEL_MAX 1811

#define CRSF_FAILURE_TIMEOUT_MS 100

typedef struct
{
    uint8_t sync_byte;
    uint8_t len;
    uint8_t type;
    uint8_t payload[CRSF_MAX_PAYLOAD_SIZE];
    uint8_t crc;
} crsf_frame_t;

typedef union {
    crsf_frame_t frame_def;
    uint8_t      bytes[CRSF_MAX_PACKET_SIZE];
} crsf_frame_packed_t;


// TODO: Add more channels
typedef struct
{
    uint16_t ch0 : 11;
    uint16_t ch1 : 11;
    uint16_t ch2 : 11;
    uint16_t ch3 : 11;
    uint16_t ch4 : 11;
} __attribute__((packed)) crsf_channels_packed_t;

typedef struct
{
    crsf_frame_packed_t crsf_frame;
    uint8_t             frame_position;
    uint8_t             frame_done;
    uint16_t            channels[CRSF_MAX_CHANNEL];
    UART_HandleTypeDef *uart;
} crsf_handle_t;

#endif //_CRSF_PROTOCOL_H_
