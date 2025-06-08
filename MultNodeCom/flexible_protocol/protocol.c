#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "protocol.h"
#include "crc16.h"

static inline uint16_t htons(uint16_t hostshort) {
    return ((hostshort & 0xFF00) >> 8) | ((hostshort & 0x00FF) << 8);
}

void proto_parser_set_callback(proto_parser_t *parser, packet_callback_t callback, void *user_data) {
    parser->callback = callback;
    parser->user_data = user_data;
}
protocol_t* proto_create_packet(uint8_t src_id, uint8_t dst_id, uint8_t cmd, uint16_t length, uint8_t* data) {
    if (length > 0 && data == NULL)
        return NULL;

    uint16_t crc16_res = 0;                                        
    size_t total_size = sizeof(protocol_t) + length + sizeof(crc16_res);
    
    protocol_t* packet = malloc(total_size);
    if (!packet) 
        return NULL;
    
    packet->head = htons(PACKET_HEAD);
    packet->src_id = src_id;
    packet->dst_id = dst_id;
    packet->cmd = cmd;
    packet->length = length;
    
    if (length > 0 && data) {
        memcpy(packet->data, data, length);
    }
    
    crc16_res = htons(crc16((const char*)packet, sizeof(protocol_t) + length));
    ((uint8_t *)packet)[total_size - 2] = crc16_res & 0xff;
    ((uint8_t *)packet)[total_size - 1] = (crc16_res >> 8) & 0xff; 

    return packet;
}

int proto_packet_free(protocol_t **packet) {
    if(*packet && packet){
        free(*packet);
        *packet = NULL;
    }else{
        printf(" packet has been free !!! \r\n");
        return -1;
    }
    return 0;
}

int proto_parser_reset(proto_parser_t *parser) {
    printf("== proto_parser_reset ==\r\n");
    if (parser->packet) {
        free(parser->packet);
        parser->packet = NULL;
    }else{
        printf(" packet has been free !!! \r\n");
        return -1;
    }
    parser->data_index = 0;
    parser->state = STATE_HEADER1;
    parser->crc = 0;
    parser->header_index = 0;
    parser->received_crc = 0;
    memset(parser->header, 0, sizeof(parser->header));
    return 0;
}
void proto_parser_init(proto_parser_t *parser) {
    printf(" proto_parser_init \r\n");
    memset(parser, 0, sizeof(proto_parser_t));
    proto_parser_reset(parser);
}

void proto_parser_destroy(proto_parser_t *parser) {
    proto_parser_reset(parser);
}

PARSE_STATUS_e proto_packet_parse(proto_parser_t* parser, uint8_t byte) {
    switch (parser->state) {
        case STATE_HEADER1:
            if (byte == ((PACKET_HEAD >> 8) & 0xFF)) {
                parser->header[parser->header_index++] = byte;
                parser->crc = crc16_update(parser->crc, byte);
                parser->state = STATE_HEADER2;
            } else {
                proto_parser_reset(parser);
                return PARSE_HEADER_ERR;
            }
            break;
            
        case STATE_HEADER2:
            if (byte == (PACKET_HEAD & 0xFF)) {
                parser->header[parser->header_index++] = byte;
                parser->crc = crc16_update(parser->crc, byte);
                parser->state = STATE_SRC_ID;
            } else {
                proto_parser_reset(parser);
                return PARSE_HEADER_ERR;
            }
            break;
            
        case STATE_SRC_ID:
            parser->header[parser->header_index++] = byte;
            parser->crc = crc16_update(parser->crc, byte);
            parser->state = STATE_DST_ID;
            break;
            
        case STATE_DST_ID:
            parser->header[parser->header_index++] = byte;
            parser->crc = crc16_update(parser->crc, byte);
            parser->state = STATE_CMD;
            break;
            
        case STATE_CMD:
            parser->header[parser->header_index++] = byte;
            parser->crc = crc16_update(parser->crc, byte);
            parser->state = STATE_LEN1;
            break;
            
        case STATE_LEN1:
            parser->header[parser->header_index++] = byte;
            parser->crc = crc16_update(parser->crc, byte);
            parser->state = STATE_LEN2;
            break;
            
        case STATE_LEN2: {
            parser->header[parser->header_index++] = byte;
            parser->crc = crc16_update(parser->crc, byte);
            
            uint16_t length = (parser->header[6] << 8) | parser->header[5];
            if (length > MAX_PACKET_SIZE) {
                proto_parser_reset(parser);
                return PARSE_LENGTH_ERR;
            }
            
            size_t total_size = GET_PACKET_LEN(length);
            parser->packet = malloc(total_size);
            if (!parser->packet) {
                proto_parser_reset(parser);
                return PARSE_MEM_ERR;
            }
            
            memcpy(parser->packet, parser->header, sizeof(parser->header));
            parser->data_index = 0;
            
            if (length > 0) {
                parser->state = STATE_DATA;
            } else {
                parser->state = STATE_CRC1;
            }
            break;
        }
            
        case STATE_DATA:
            // 主动检测包头特征 (0x5A后接0xA5)
            if (parser->prev_byte == ((PACKET_HEAD >> 8) & 0xFF) && byte == (PACKET_HEAD & 0xFF)) {
                proto_parser_reset(parser);
                
                parser->crc = crc16_update(parser->crc, ((PACKET_HEAD >> 8) & 0xFF));
                parser->header[parser->header_index++] = ((PACKET_HEAD >> 8) & 0xFF);
                
                parser->crc = crc16_update(parser->crc, (PACKET_HEAD & 0xFF));
                parser->header[parser->header_index++] = (PACKET_HEAD & 0xFF);
                
                parser->state = STATE_SRC_ID;
                parser->prev_byte = 0; 
                break;
            }
            parser->prev_byte = byte;

            if (parser->data_index >= MAX_PACKET_SIZE) {
                proto_parser_reset(parser);
                return PARSE_LENGTH_ERR;
            }

            parser->packet->data[parser->data_index++] = byte;
            parser->crc = crc16_update(parser->crc, byte);

            if (parser->data_index >= parser->packet->length) {
                parser->state = STATE_CRC1;
            }

            break;
            
        case STATE_CRC1:
            parser->received_crc = (uint16_t)byte << 8;
            parser->state = STATE_CRC2;
            break;
            
        case STATE_CRC2: {
            parser->received_crc |= byte;        

            if (parser->received_crc != parser->crc) {
                proto_parser_reset(parser);
                return PARSE_CRC_ERR;
            }

            if (parser->callback) {
                parser->callback(parser->packet, parser->user_data);
            }
            proto_parser_reset(parser);
            return PARSE_OK;
        }
    }

    return PARSE_INCOMPLETE;
}