/*
 * REF: https://github.com/crsf-wg/crsf/wiki/Message-Format
 * Format: [address] [len] [type] [payload] [crc8]
 * */

// frame type
// địa chỉ
// max length

#ifndef _CRSF_PROTOCOL_H_
#define _CRSF_PROTOCOL_H_

#include <stdint.h>

#define CRSF_MAX_CHANNEL 5 // total 16 channels
#define CRSF_BITS_PER_CHANNEL 11
#define CRSF_MAX_PAYLOAD_SIZE 60
#define CRSF_MAX_PACKET_SIZE (CRSF_MAX_PAYLOAD_SIZE + 4) // PayloadLength + 4 (sync, len, type, crc)

#define CRSF_SYNC_BYTE (0xC8)                    // Address: Flight Controller
#define CRSF_FRAMETYPE_RC_CHANNELS_PACKED (0x16) // Frame types

#define CRSF_VALUE_CHANNEL_MIN 172
#define CRSF_VALUE_CHANNEL_MID 992
#define CRSF_VALUE_CHANNEL_MAX 1811

#define CRSF_FAILURE_TIMEOUT_MS 500 

typedef struct
{
    uint8_t sync_byte;
    uint8_t len;
    uint8_t type;
    uint8_t payload[CRSF_MAX_PAYLOAD_SIZE];
    uint8_t crc;
} crsf_frame_t;

// TODO: Add more channels
typedef struct
{
    uint16_t ch0;
    uint16_t ch1;
    uint16_t ch2;
    uint16_t ch3;
    uint16_t ch4;
} crsf_channels_t;

#endif //_CRSF_PROTOCOL_H_
