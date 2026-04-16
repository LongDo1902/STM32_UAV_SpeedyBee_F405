//đọc buffer DMA
//parse frame
//verify CRC

#ifndef _CRSF_PARSER_H_
#define _CRSF_PARSER_H_

#include <stdint.h>
#include <stdbool.h>

#include "crsf_protocol.h"

typedef enum{
    CRSF_STATE_SYNC,
    CRSF_STATE_LEN,
    CRSF_STATE_TYPE,
    CRSF_STATE_PAYLOAD,
    CRSF_STATE_CRC
}crsf_state_t;

typedef struct{
    crsf_state_t state;
    crsf_frame_t frame;
    uint8_t frame_position;
    uint8_t frame_done;
}crsf_parser_t;

void crsf_parser_init(crsf_parser_t *parser);

int8_t crsf_parse_byte(crsf_parser_t *parser, uint8_t byte);

void crsf_decode_channels(const uint8_t *payload, uint16_t *channels);

// TODO: Implement check crc when receive each byte
uint8_t crsf_compute_crc(const uint8_t *data, uint8_t len);

#endif // _CRSF_PARSER_H_
