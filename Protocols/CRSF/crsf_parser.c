#include "crsf_parser.h"

//extern crsf_handle_t crsf_handle;

void crsf_parser_init(crsf_parser_t *parser){
    parser->state = CRSF_STATE_SYNC;
}

int8_t crsf_parse_byte(crsf_parser_t *parser, uint8_t byte){
    switch (parser->state) {

        case CRSF_STATE_SYNC:
            if(byte == CRSF_SYNC_BYTE){
                parser->frame.sync_byte = byte;
                parser->state = CRSF_STATE_LEN;
            }    

            break;
        case CRSF_STATE_LEN:
            if(byte < 2 || byte >CRSF_MAX_PAYLOAD_SIZE + 2){
                parser->state = CRSF_STATE_SYNC;
                break;
            }
            parser->frame.len = byte;
            parser->frame.crc = 0;
            parser->frame_position = 0;
            parser->state = CRSF_STATE_TYPE;
            
            break;
        case CRSF_STATE_TYPE:
            parser->frame.type = byte;
            parser->state = (parser->frame.len > 2) ? CRSF_STATE_PAYLOAD:CRSF_STATE_CRC; 

            break;        
        case CRSF_STATE_PAYLOAD:
            
            parser->frame.payload[parser->frame_position++] = byte;
            // payload length = len - 2 (type byte + crc byte)
            if (parser->frame_position >= (parser->frame.len - 2)) {
                parser->state = CRSF_STATE_CRC;
            }
            break;
    
        case CRSF_STATE_CRC:
            
            uint8_t crc = crsf_compute_crc(&parser->frame.type, parser->frame.len - 1);
            if(byte == crc ){
                parser->frame.crc = byte;
                parser->frame_done = 1;
            }

            parser->state = CRSF_STATE_SYNC;
            return parser->frame_done;

        default:
            parser->state = CRSF_STATE_SYNC;
            break;
    }

    return false;
}

void crsf_handle_rc_channels_packed(const uint8_t *payload, uint16_t *channels){
    
    const crsf_channels_t *packed = (const crsf_channels_t *)payload;

    channels[0] = packed->ch0;
    channels[1] = packed->ch1;
    channels[2] = packed->ch2;
    channels[3] = packed->ch3;
    channels[4] = packed->ch4;
}

uint8_t crsf_compute_crc(const uint8_t *data, uint8_t len){
    uint8_t crc = 0;

    for(uint8_t i = 0; i < len; i++){
        crc ^= data[i];

        for(int j = 0; j < 8; j++)
        {
            if(crc &0x80){
                crc = (crc << 0x01) ^ 0xD5;
            }
            else{
                crc = crc << 1;
            }
        }
    }

    return crc;
}

