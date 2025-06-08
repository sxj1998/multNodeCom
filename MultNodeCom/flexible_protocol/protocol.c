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
    memset(parser, 0, sizeof(proto_parser_t));
    proto_parser_reset(parser);
}

void proto_parser_destroy(proto_parser_t *parser) {
    proto_parser_reset(parser);
}

PARSE_STATUS_e proto_packet_parse(proto_parser_t* parser, uint8_t byte) {
    printf("recv byte: %02x \r\n", byte);
    switch (parser->state) {
        case STATE_HEADER1:
            printf("STATE_HEADER1 ");
            if (byte == ((PACKET_HEAD >> 8) & 0xFF)) {
                printf("=> 匹配包头高位 (0x%02X)\n", (PACKET_HEAD >> 8) & 0xFF);
                parser->header[parser->header_index++] = byte;
                parser->crc = crc16_update(parser->crc, byte);
                parser->state = STATE_HEADER2;
            } else {
                printf("=> 包头高位不匹配 (期望0x%02X)\n", (PACKET_HEAD >> 8) & 0xFF);
                proto_parser_reset(parser);
                return PARSE_HEADER_ERR;
            }
            break;
            
        case STATE_HEADER2:
            printf("STATE_HEADER2 ");
            if (byte == (PACKET_HEAD & 0xFF)) {
                printf("=> 匹配包头低位 (0x%02X)\n", PACKET_HEAD & 0xFF);
                parser->header[parser->header_index++] = byte;
                parser->crc = crc16_update(parser->crc, byte);
                parser->state = STATE_SRC_ID;
            } else {
                printf("=> 包头低位不匹配 (期望0x%02X)\n", PACKET_HEAD & 0xFF);
                proto_parser_reset(parser);
                return PARSE_HEADER_ERR;
            }
            break;
            
        case STATE_SRC_ID:
            printf("STATE_SRC_ID ");
            parser->header[parser->header_index++] = byte;
            parser->crc = crc16_update(parser->crc, byte);
            parser->state = STATE_DST_ID;
            printf("=> src_id = 0x%02X\n", byte);
            break;
            
        case STATE_DST_ID:
            printf("STATE_DST_ID ");
            parser->header[parser->header_index++] = byte;
            parser->crc = crc16_update(parser->crc, byte);
            parser->state = STATE_CMD;
            printf("=> dst_id = 0x%02X\n", byte);
            break;
            
        case STATE_CMD:
            printf("STATE_CMD ");
            parser->header[parser->header_index++] = byte;
            parser->crc = crc16_update(parser->crc, byte);
            parser->state = STATE_LEN1;
            printf("=> cmd = 0x%02X\n", byte);
            break;
            
        case STATE_LEN1:
            printf("STATE_LEN1 ");
            parser->header[parser->header_index++] = byte;
            parser->crc = crc16_update(parser->crc, byte);
            parser->state = STATE_LEN2;
            printf("=> len_high = 0x%02X\n", byte);
            break;
            
        case STATE_LEN2: {
            printf("STATE_LEN2 ");
            parser->header[parser->header_index++] = byte;
            parser->crc = crc16_update(parser->crc, byte);
            
            uint16_t length = (parser->header[6] << 8) | parser->header[5];
            printf("=> len_low = 0x%02X | 总长度 = %u\n", byte, length);
            if (length > MAX_PACKET_SIZE) {
                printf("=> 长度超过限制 (MAX_PACKET_SIZE = %d)\n", MAX_PACKET_SIZE);
                proto_parser_reset(parser);
                return PARSE_LENGTH_ERR;
            }
            
            size_t total_size = GET_PACKET_LEN(length);
            parser->packet = malloc(total_size);
            if (!parser->packet) {
                printf("=> 内存分配失败 (%zu bytes)\n", total_size);
                proto_parser_reset(parser);
                return PARSE_MEM_ERR;
            }
            
            memcpy(parser->packet, parser->header, sizeof(parser->header));
            parser->data_index = 0;
            
            printf("=> 分配内存成功 (%zu bytes) | 数据状态: ", total_size);
            if (length > 0) {
                parser->state = STATE_DATA;
                printf("STATE_DATA (将接收%d字节)\n", length);
            } else {
                parser->state = STATE_CRC1;
                printf("STATE_CRC1 (无数据)\n");
            }
            break;
        }
            
        case STATE_DATA:
            printf("STATE_DATA ");
            // 主动检测包头特征 (0x5A后接0xA5)
            if (parser->prev_byte == ((PACKET_HEAD >> 8) & 0xFF) && byte == (PACKET_HEAD & 0xFF)) {
                printf("=> 检测到异常包头，启动快速恢复\n");
                proto_parser_reset(parser);
                
                parser->crc = crc16_update(parser->crc, ((PACKET_HEAD >> 8) & 0xFF));
                parser->header[parser->header_index++] = ((PACKET_HEAD >> 8) & 0xFF);
                
                parser->crc = crc16_update(parser->crc, (PACKET_HEAD & 0xFF));
                parser->header[parser->header_index++] = (PACKET_HEAD & 0xFF);
                
                parser->state = STATE_SRC_ID;
                printf("=> 解析器已重置并处理新包头\n");
                parser->prev_byte = 0; // 重置前字节记录
                break;
            }
            // 记录当前字节用于下次检测
            parser->prev_byte = byte;

            if (parser->data_index >= MAX_PACKET_SIZE) {
                printf("=> 数据索引越界 (index=%d, max=%d)\n", 
                      parser->data_index, MAX_PACKET_SIZE);
                proto_parser_reset(parser);
                return PARSE_LENGTH_ERR;
            }

            parser->packet->data[parser->data_index++] = byte;
            parser->crc = crc16_update(parser->crc, byte);
            
            printf("=> data[%d] = 0x%02X | CRC=0x%04X\n", 
                  parser->data_index - 1, byte, parser->crc);

            if (parser->data_index >= parser->packet->length) {
                parser->state = STATE_CRC1;
                printf("=> 数据接收完成 转至STATE_CRC1\n");
            }

            break;
            
        case STATE_CRC1:
            printf("STATE_CRC1 ");
            parser->received_crc = (uint16_t)byte << 8;
            parser->state = STATE_CRC2;
            printf("=> CRC高字节 = 0x%02X\n", byte);
            break;
            
        case STATE_CRC2: {
            printf("STATE_CRC2 ");
            parser->received_crc |= byte;
            printf("=> CRC低字节 = 0x%02X | 接收CRC = 0x%04X | 计算CRC = 0x%04X\n",
                  byte, parser->received_crc, parser->crc);            

            if (parser->received_crc != parser->crc) {
                printf("=> CRC校验失败 \n");
                proto_parser_reset(parser);
                return PARSE_CRC_ERR;
            }
            printf("=> CRC校验成功 完整包接收完毕\n");

            if (parser->callback) {
                parser->callback(parser->packet, parser->user_data);
            }
            proto_parser_reset(parser);
            return PARSE_OK;
        }

    }

    return PARSE_INCOMPLETE;
}