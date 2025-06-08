#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "protocol.h"
#include "crc16.h"
#include "utilsPrintf.h"

// 创建有损信道测试
void test_lossy_channel() {
    proto_parser_t parser;
    proto_parser_init(&parser);
    
    // 构建包含异常包头的数据流
    uint8_t stream[] = {
        // 受损包开始
        0x5A, 0xA5, 0x01, 0x02, 0x03, 
        // 长度字段声明100字节 (正常范围)
        0x64, 0x00, 
        // 实际只有10字节数据
        0x11, 0x22, 0x33, 0x44, 0x55,
        // 嵌入新包头 (后续有效数据)
        0x5A, 0xA5, // 被检测到→重置
        0xAA, 0xBB, 0xCC, 0x03, 0x00, 
        0xDD, 0xEE, 0xFF, 0x12, 0x34
    };
    
    for (int i = 0; i < sizeof(stream); i++) {
        PARSE_STATUS_e s = proto_packet_parse(&parser, stream[i]);
        if (s == PARSE_OK) {
            printf("[成功] 接收到有效包!\n");
            printf("源地址: 0x%02X, 目标: 0x%02X, 数据长度: %d\n",
                  parser.packet->src_id, 
                  parser.packet->dst_id,
                  parser.packet->length);
        }
    }
}

int test_normal(){
        // 创建解析器
    proto_parser_t parser;
    proto_parser_init(&parser);
    
    // 创建测试包
    uint8_t test_data[] = {0x01, 0x02, 0x03};
    protocol_t* packet = proto_create_packet(0x01, 0x02, 0x03, 
                                           sizeof(test_data), test_data);
    if (!packet) {
        printf("Packet creation failed\n");
        return 1;
    }
    
    // 获取包数据
    size_t packet_len = GET_PACKET_LEN(packet->length);
    uint8_t* packet_data = (uint8_t*)packet;
    
    // 解析每个字节
    for (size_t i = 0; i < packet_len; i++) {
        PARSE_STATUS_e status = proto_packet_parse(&parser, packet_data[i]);
        switch (status) {
            case PARSE_OK:
                printf("Packet parsed successfully!\n");
                printf("Source: 0x%02X, Destination: 0x%02X, Cmd: 0x%02X\n",
                       parser.packet->src_id, parser.packet->dst_id, parser.packet->cmd);
                printf("Data length: %u\n", parser.packet->length);
                break;
            case PARSE_HEADER_ERR:
                printf("Header error\n");
                break;
            case PARSE_CRC_ERR:
                printf("CRC error\n");
                break;
            case PARSE_MEM_ERR:
                printf("Memory error\n");
                break;
            case PARSE_LENGTH_ERR:
                printf("Length error\n");
                break;
            default:
                // PARSE_INCOMPLETE - 继续解析
                break;
        }
    }
    
    proto_packet_free(&parser.packet);
    // 清理资源
    proto_packet_free(&packet);
    proto_parser_destroy(&parser);
}

int main() {

    // test_lossy_channel();
    test_normal();
    
    return 0;
}


