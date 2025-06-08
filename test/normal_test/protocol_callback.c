#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "protocol.h"
#include "crc16.h"
#include "utilsPrintf.h"

// 回调函数示例
void my_packet_handler(protocol_t *packet, void *user_data) {
    printf("\n=== 回调触发 ===\n");
    printf("源地址: 0x%02X, 目标: 0x%02X, 命令: 0x%02X, 长度: %u\n",
           packet->src_id, packet->dst_id, packet->cmd, packet->length);
    
    // 处理数据...
    if (packet->length > 0) {
        printf("数据: ");
        for (int i = 0; i < packet->length; i++) {
            printf("%02X ", packet->data[i]);
        }
        printf("\n");
    }
}

int main() {
    proto_parser_t parser;
    proto_parser_init(&parser);
    
    // 设置回调函数
    proto_parser_set_callback(&parser, my_packet_handler, NULL);
    
    // 模拟接收数据
    uint8_t test_data[] = {0x01, 0x02, 0x03};
    protocol_t* packet = proto_create_packet(0xAA, 0xBB, 0x03, 
                                           sizeof(test_data), test_data);
    
    size_t packet_len = GET_PACKET_LEN(packet->length);
    uint8_t data[1024] = {};
    memcpy(data, packet, packet_len);
    
    for (int i = 0; i < packet_len; i++) {
        PARSE_STATUS_e status = proto_packet_parse(&parser, data[i]);
        if (status == PARSE_OK) {
            printf("主循环收到解析完成通知\n");
        }else {
        }
    }
        
    return 0;
}