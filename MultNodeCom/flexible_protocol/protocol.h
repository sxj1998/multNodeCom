#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdint.h>
#include <stdbool.h>

#define PACKET_HEAD 0x5AA5
#define MAX_PACKET_SIZE 1024  // 最大包大小限制

/* 消息头大小 + 数据包大小 + crc*/
#define GET_PACKET_LEN(data_length) ( sizeof(protocol_t) + data_length + sizeof(uint16_t) )
#define GET_PACKET_LEN_WITHOUT_HEAD(data_length) ( data_length + sizeof(uint16_t) )

typedef struct protocol_s protocol_t;
typedef void (*packet_callback_t)(protocol_t *packet, void *user_data);

typedef enum {
    PARSE_INCOMPLETE,   // 解析未完成
    PARSE_OK,           // 解析成功
    PARSE_HEADER_ERR,   // 包头错误
    PARSE_LENGTH_ERR,
    PARSE_CRC_ERR,      // CRC校验失败
    PARSE_MEM_ERR       // 内存错误
} PARSE_STATUS_e;

// 解析器状态定义
typedef enum {
    STATE_HEADER1,  // 等待包头高字节
    STATE_HEADER2,  // 等待包头低字节
    STATE_SRC_ID,   // 等待源地址
    STATE_DST_ID,   // 等待目的地址
    STATE_CMD,      // 等待命令
    STATE_LEN1,     // 等待长度高字节
    STATE_LEN2,     // 等待长度低字节
    STATE_DATA,     // 等待数据
    STATE_CRC1,     // 等待CRC高字节
    STATE_CRC2      // 等待CRC低字节
} PARSE_STATE_e;

#pragma pack (1)  
typedef struct protocol_s{
    uint16_t head;          //0x5AA5
    uint8_t src_id;         //源地址
    uint8_t dst_id;         //目的地址
    uint8_t cmd;            //命令
    uint16_t length;        //数据长度
    uint8_t data[];         //data 最后两字节包含crc
} protocol_t;
#pragma pack () 

typedef struct {
    PARSE_STATE_e state;    // 当前解析状态
    protocol_t* packet;     // 解析中的协议包
    uint16_t data_index;    // 当前数据索引
    uint16_t crc;           // 当前计算的CRC值
    uint16_t received_crc;  // 接收到的CRC值
    uint8_t header[7];      // 包头缓存 (head+src+dst+cmd+len)
    uint8_t header_index;   // 包头缓存索引
    uint8_t prev_byte;      // 用于异常包检测
    packet_callback_t callback;
    void *user_data;
} proto_parser_t;

protocol_t* proto_create_packet(uint8_t src_id, uint8_t dst_id, uint8_t cmd, uint16_t length, uint8_t* data);

int proto_packet_free(protocol_t **packet);

void proto_parser_init(proto_parser_t *parser);

void proto_parser_destroy(proto_parser_t *parser);

int proto_parser_reset(proto_parser_t *parser);

PARSE_STATUS_e proto_packet_parse(proto_parser_t* parser, uint8_t byte) ;

void proto_parser_set_callback(proto_parser_t *parser, packet_callback_t callback, void *user_data);


#endif