/* ================= protocol_opt.h ================= */
#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdint.h>
#include <stdlib.h>

/* 配置选项 */
#define PACKET_HEAD 0x5AA5
#define MAX_PACKET_SIZE 512

/* 字节序转换 */
#if defined(__BYTE_ORDER__) && (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
#define PROTO_HTONS(n) (n)
#define PROTO_NTOHS(n) (n)
#else
#define PROTO_HTONS(n) ((((n) & 0xFF00) >> 8) | (((n) & 0x00FF) << 8))
#define PROTO_NTOHS(n) PROTO_HTONS(n)
#endif

/* 完整包大小 = 协议头 + 数据长度 + CRC大小 */
#define GET_PACKET_LEN(data_length) (sizeof(protocol_t) + (data_length) + sizeof(uint16_t))

#define PACKET_SEND_LEN(packet) (proto_escaped_length(packet))
/* 数据包回调类型定义 */
typedef void (*packet_callback_t)(void* packet, void* user_data);

/* 解析状态枚举 */
typedef enum {
    PARSE_OK,               // 解析成功
    PARSE_INCOMPLETE,       // 解析未完成
    PARSE_ERROR_HEADER,     // 包头错误
    PARSE_ERROR_LENGTH,     // 长度错误
    PARSE_ERROR_CRC,        // CRC校验失败
    PARSE_ERROR_MEMORY,     // 内存错误
    PARSE_ERROR_INTERNAL,   // 内部错误
    PARSE_ERROR_ESCAPE      // 转义错误
} PARSE_STATUS;

/* 状态机状态枚举 */
typedef enum {
    STATE_HEADER1,          // 等待包头高字节
    STATE_HEADER2,          // 等待包头低字节
    STATE_INDEX1,            // 等待包索引
    STATE_INDEX2,            // 等待包索引
    STATE_SRC_ID,           // 等待源地址
    STATE_DST_ID,           // 等待目的地址
    STATE_CMD,              // 等待命令
    STATE_LEN1,             // 等待长度高字节
    STATE_LEN2,             // 等待长度低字节
    STATE_DATA,             // 等待数据
    STATE_ESCAPE,           // 转义序列处理状态
    STATE_CRC1,             // 等待CRC高字节
    STATE_CRC2,             // 等待CRC低字节
    STATE_COUNT             // 状态总数
} PROTO_STATE;

/* 协议结构体 */
#pragma pack(push, 1)
typedef struct {
    uint16_t head;          // 固定包头 0x5AA5
    uint16_t index;         // 包索引
    uint8_t src_id;         // 源地址
    uint8_t dst_id;         // 目的地址
    uint8_t cmd;            // 命令字
    uint16_t length;        // 数据长度 (网络字节序)
    uint8_t data[];         // 有效载荷
} protocol_t;
#pragma pack(pop)

/* 解析器上下文结构 */
typedef struct {
    PROTO_STATE state;      // 当前解析状态
    protocol_t* packet;     // 当前解析的数据包
    
    /* 解析状态临时变量 */
    uint16_t expected_len;  // 期望数据长度
    uint16_t data_index;    // 当前数据索引
    uint16_t crc;           // 当前计算的CRC值
    uint16_t received_crc;  // 接收到的CRC值
    uint16_t index;         // 临时存储索引
    uint8_t src_id;         // 临时存储源地址
    uint8_t dst_id;         // 临时存储目的地址
    uint8_t cmd;            // 临时存储命令字
    
    packet_callback_t callback; // 数据包回调函数
    void* user_data;        // 用户自定义数据
} proto_parser_t;

/* 接口函数声明 */
void* proto_create_packet(uint8_t src_id, uint8_t dst_id, uint8_t cmd, uint16_t length, const uint8_t* data);
void* proto_create_packet_with_index(uint8_t src_id, uint8_t dst_id, uint8_t cmd, 
                                   uint16_t length, const uint8_t* data, uint16_t index);
void proto_packet_free(void** packet);
void proto_parser_init(proto_parser_t* parser);
void proto_parser_destroy(proto_parser_t* parser);
void proto_parser_reset(proto_parser_t* parser);
void proto_parser_set_callback(proto_parser_t* parser, packet_callback_t callback, void* user_data);
PARSE_STATUS proto_packet_parse(proto_parser_t* parser, uint8_t byte);
int proto_escaped_length(protocol_t* packet);

#endif // PROTOCOL_H