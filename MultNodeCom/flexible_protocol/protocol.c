/* ================= protocol.c ================= */
#include "protocol.h"
#include "crc16.h"
#include "xlog.h"
#include <string.h>

#define ESCAPE_CHAR 0x5B
#define ESCAPE_HEADER_HIGH 0x01
#define ESCAPE_HEADER_LOW 0x02

/* 状态处理函数声明 */
static PARSE_STATUS handle_header1(proto_parser_t* parser, uint8_t byte);
static PARSE_STATUS handle_header2(proto_parser_t* parser, uint8_t byte);
static PARSE_STATUS handle_src_id(proto_parser_t* parser, uint8_t byte);
static PARSE_STATUS handle_dst_id(proto_parser_t* parser, uint8_t byte);
static PARSE_STATUS handle_index1(proto_parser_t* parser, uint8_t byte);
static PARSE_STATUS handle_index2(proto_parser_t* parser, uint8_t byte);
static PARSE_STATUS handle_cmd(proto_parser_t* parser, uint8_t byte);
static PARSE_STATUS handle_len1(proto_parser_t* parser, uint8_t byte);
static PARSE_STATUS handle_len2(proto_parser_t* parser, uint8_t byte);
static PARSE_STATUS handle_data(proto_parser_t* parser, uint8_t byte);
static PARSE_STATUS handle_crc1(proto_parser_t* parser, uint8_t byte);
static PARSE_STATUS handle_crc2(proto_parser_t* parser, uint8_t byte);

/* 状态处理函数指针数组 */
static PARSE_STATUS (*const state_handlers[])(proto_parser_t*, uint8_t) = {
    handle_header1,  // STATE_HEADER1
    handle_header2,  // STATE_HEADER2
    handle_index1,   // STATE_INDEX1
    handle_index2,   // STATE_INDEX2
    handle_src_id,   // STATE_SRC_ID
    handle_dst_id,   // STATE_DST_ID
    handle_cmd,      // STATE_CMD
    handle_len1,     // STATE_LEN1
    handle_len2,     // STATE_LEN2
    handle_data,     // STATE_DATA
    handle_crc1,     // STATE_CRC1
    handle_crc2      // STATE_CRC2
};

/* ================= 公共接口函数 ================= */
static size_t calculate_escaped_length(const uint8_t* src, size_t len) {
    size_t escaped_len = len;
    for (size_t i = 0; i < len; i++) {
        if (src[i] == ((PACKET_HEAD >> 8) & 0xFF) || 
            src[i] == (PACKET_HEAD & 0xFF) || 
            src[i] == ESCAPE_CHAR) {
            escaped_len++;  // 每个需要转义的字符增加1字节
        }
    }
    return escaped_len;
}

// 执行数据转义
static void escape_data(uint8_t* dest, const uint8_t* src, size_t len, size_t* escaped_len) {
    size_t j = 0;
    for (size_t i = 0; i < len; i++) {
        if (src[i] == ((PACKET_HEAD >> 8) & 0xFF)) {
            dest[j++] = ESCAPE_CHAR;
            dest[j++] = ESCAPE_HEADER_HIGH;
        } else if (src[i] == (PACKET_HEAD & 0xFF)) {
            dest[j++] = ESCAPE_CHAR;
            dest[j++] = ESCAPE_HEADER_LOW;
        } else if (src[i] == ESCAPE_CHAR) {
            dest[j++] = ESCAPE_CHAR;
            dest[j++] = ESCAPE_CHAR;
        } else {
            dest[j++] = src[i];
        }
    }
    *escaped_len = j;
}

/* 创建数据包 */
void* proto_create_packet(uint8_t src_id, uint8_t dst_id, uint8_t cmd, uint16_t length, const uint8_t* data) {
    static uint16_t index = 0;
    return proto_create_packet_with_index(src_id, dst_id, cmd, length, data, index++);
}

/* 创建带索引的数据包 */
void* proto_create_packet_with_index(uint8_t src_id, uint8_t dst_id, uint8_t cmd, 
                                   uint16_t length, const uint8_t* data, uint16_t index) {
    if (length > MAX_PACKET_SIZE) {
        LOG_ERROR("[PROTO] Error: Packet len %u > MAX %u", length, MAX_PACKET_SIZE);
        return NULL;
    }
    
    if (length > 0 && !data) {
        LOG_ERROR("[PROTO] Error: Data required for non-zero len");
        return NULL;
    }

    // 计算转义后的数据长度
    size_t escaped_len = (length > 0) ? calculate_escaped_length(data, length) : 0;
    LOG_INFO("escaped_len: %d , raw length %d , escape char num: %d ", escaped_len, length, escaped_len-length);

    size_t total_size = GET_PACKET_LEN(escaped_len);
    protocol_t* packet = malloc(total_size);
    if (!packet) {
        LOG_ERROR("[PROTO] Error: Allocation failed size %zu", total_size);
        return NULL;
    }
    
    packet->head = PROTO_HTONS(PACKET_HEAD);
    packet->index = PROTO_HTONS(index);
    packet->src_id = src_id;
    packet->dst_id = dst_id;
    packet->cmd = cmd;
    packet->length = PROTO_HTONS(length);
    
    uint8_t tmp_print[512] = {};
    memcpy(tmp_print, packet, total_size);
    LOG_BYTE_ARRAY(LOG_LEVEL_INFO, (uint8_t*)tmp_print, total_size); 

    // 转义数据并复制到包中
    if (length > 0) {
        uint8_t* data_dest = packet->data;
        escape_data(data_dest, data, length, &escaped_len);
    }
    uint16_t crc = crc16((const char*)packet, sizeof(protocol_t) + length);
    uint16_t net_crc = PROTO_HTONS(crc);
    memcpy(packet->data + length, &net_crc, sizeof(net_crc));
    
    LOG_INFO("[PROTO] Created: src=%u, dst=%u, index=%u, cmd=%u, len=%u", 
           src_id, dst_id, index, cmd, length);
 
    return packet;
}

/* 释放数据包 */
void proto_packet_free(void** packet_ptr) {
    if (!packet_ptr || !*packet_ptr) return;
    free(*packet_ptr);
    *packet_ptr = NULL;
}

/* 初始化解析器 */
void proto_parser_init(proto_parser_t* parser) {
    if (!parser) return;
    
    memset(parser, 0, sizeof(proto_parser_t));
    parser->crc = 0xFFFF;
}

/* 销毁解析器 */
void proto_parser_destroy(proto_parser_t* parser) {
    if (!parser) return;
    proto_parser_reset(parser);
}

/* 重置解析器状态 */
/* 重置解析器状态 */
void proto_parser_reset(proto_parser_t* parser) {
    if (!parser) return;
    
    if (parser->packet) {
        free(parser->packet);
        parser->packet = NULL;
    }
    
    parser->state = STATE_HEADER1;
    parser->expected_len = 0;
    parser->data_index = 0;
    parser->src_id = 0;
    parser->dst_id = 0;
    parser->cmd = 0;
    parser->crc = 0xFFFF;
    parser->received_crc = 0;
}

/* 设置数据包回调函数 */
void proto_parser_set_callback(proto_parser_t* parser, packet_callback_t callback, void* user_data) {
    if (!parser) return;
    parser->callback = callback;
    parser->user_data = user_data;
}

/* ================= 解析状态机函数 ================= */

static PARSE_STATUS handle_header1(proto_parser_t* parser, uint8_t byte) {
    if (byte == ((PACKET_HEAD >> 8) & 0xFF)) {
        parser->crc = crc16_update(parser->crc, byte);
        parser->state = STATE_HEADER2;
        return PARSE_INCOMPLETE;
    }
    
    LOG_WARN("Header1 exp 0x%02X got 0x%02X", (PACKET_HEAD >> 8) & 0xFF, byte);
    proto_parser_reset(parser);
    return PARSE_ERROR_HEADER;
}

static PARSE_STATUS handle_header2(proto_parser_t* parser, uint8_t byte) {
    if (byte == (PACKET_HEAD & 0xFF)) {
        parser->crc = crc16_update(parser->crc, byte);
        parser->state = STATE_INDEX1;
        return PARSE_INCOMPLETE;
    }
    
    LOG_WARN("Header2 exp 0x%02X got 0x%02X", PACKET_HEAD & 0xFF, byte);
    proto_parser_reset(parser);
    return PARSE_ERROR_HEADER;
}

static PARSE_STATUS handle_index1(proto_parser_t* parser, uint8_t byte){
    parser->index = (uint16_t)byte << 8;
    parser->crc = crc16_update(parser->crc, byte);
    parser->state = STATE_INDEX2;
    return PARSE_INCOMPLETE;
}

static PARSE_STATUS handle_index2(proto_parser_t* parser, uint8_t byte){
    parser->index |= byte;
    parser->crc = crc16_update(parser->crc, byte);
    parser->state = STATE_SRC_ID;
    return PARSE_INCOMPLETE;
}

static PARSE_STATUS handle_src_id(proto_parser_t* parser, uint8_t byte) {
    parser->src_id = byte;
    parser->crc = crc16_update(parser->crc, byte);
    parser->state = STATE_DST_ID;
    return PARSE_INCOMPLETE;
}

static PARSE_STATUS handle_dst_id(proto_parser_t* parser, uint8_t byte) {
    parser->dst_id = byte;
    parser->crc = crc16_update(parser->crc, byte);
    parser->state = STATE_CMD;
    return PARSE_INCOMPLETE;
}

static PARSE_STATUS handle_cmd(proto_parser_t* parser, uint8_t byte) {
    parser->cmd = byte;
    parser->crc = crc16_update(parser->crc, byte);
    parser->state = STATE_LEN1;
    return PARSE_INCOMPLETE;
}

static PARSE_STATUS handle_len1(proto_parser_t* parser, uint8_t byte) {
    parser->expected_len = (uint16_t)byte << 8;
    parser->crc = crc16_update(parser->crc, byte);
    parser->state = STATE_LEN2;
    return PARSE_INCOMPLETE;
}

static PARSE_STATUS handle_len2(proto_parser_t* parser, uint8_t byte) {
    parser->expected_len |= byte;
    parser->crc = crc16_update(parser->crc, byte);
    
    if (parser->expected_len > MAX_PACKET_SIZE) {
        LOG_ERROR("Invalid len %u > MAX %u", parser->expected_len, MAX_PACKET_SIZE);
        proto_parser_reset(parser);
        return PARSE_ERROR_LENGTH;
    }
    
    if (parser->expected_len > 0) {
        parser->state = STATE_DATA;
    } else {
        parser->state = STATE_CRC1;
    }
    
    parser->packet = malloc(GET_PACKET_LEN(parser->expected_len));
    if (!parser->packet) {
        LOG_ERROR("Alloc failed len %u", parser->expected_len);
        proto_parser_reset(parser);
        return PARSE_ERROR_MEMORY;
    }
    
    /* 填充已知字段 */
    parser->packet->head = PROTO_HTONS(PACKET_HEAD);
    parser->packet->index = parser->index;
    parser->packet->src_id = parser->src_id;
    parser->packet->dst_id = parser->dst_id;
    parser->packet->cmd = parser->cmd;
    parser->packet->length = PROTO_HTONS(parser->expected_len);
    
    parser->data_index = 0;
    return PARSE_INCOMPLETE;
}

static PARSE_STATUS handle_data(proto_parser_t* parser, uint8_t byte) {
    if (parser->data_index >= parser->expected_len) {
        LOG_ERROR("Data index %u overflow", parser->data_index);
        proto_parser_reset(parser);
        return PARSE_ERROR_LENGTH;
    }
    
    parser->packet->data[parser->data_index++] = byte;
    parser->crc = crc16_update(parser->crc, byte);
    
    if (parser->data_index >= parser->expected_len) {
        parser->state = STATE_CRC1;
    }
    
    return PARSE_INCOMPLETE;
}

static PARSE_STATUS handle_crc1(proto_parser_t* parser, uint8_t byte) {
    parser->received_crc = (uint16_t)byte << 8;
    parser->state = STATE_CRC2;
    return PARSE_INCOMPLETE;
}

static PARSE_STATUS handle_crc2(proto_parser_t* parser, uint8_t byte) {
    parser->received_crc |= byte;
    
    if (parser->received_crc != parser->crc) {
        LOG_ERROR("CRC exp 0x%04X got 0x%04X", parser->crc, parser->received_crc);
        proto_parser_reset(parser);
        return PARSE_ERROR_CRC;
    }
    
    /* 所有权转移给回调 */
    protocol_t* completed = parser->packet;
    parser->packet = NULL;
    
    if (parser->callback) {
        parser->callback(completed, parser->user_data);
    } else {
        free(completed);
    }
    
    proto_parser_reset(parser);
    return PARSE_OK;
}

/* 主解析函数 */
PARSE_STATUS proto_packet_parse(proto_parser_t* parser, uint8_t byte) {
    if (!parser) return PARSE_ERROR_INTERNAL;
    if (parser->state >= sizeof(state_handlers)/sizeof(state_handlers[0])) {
        LOG_ERROR("Invalid state %d", parser->state);
        proto_parser_reset(parser);
        return PARSE_ERROR_INTERNAL;
    }
    return state_handlers[parser->state](parser, byte);
}