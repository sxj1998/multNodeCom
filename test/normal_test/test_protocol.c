#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "protocol.h"
#include "utils/crc16.h"

// 日志回调
void log_handler(PROTO_LOG_LEVEL level, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    
    const char* level_str = "";
    switch(level) {
        case PROTO_LOG_DEBUG: level_str = "[DEBUG] "; break;
        case PROTO_LOG_WARNING: level_str = "[WARN] "; break;
        case PROTO_LOG_ERROR: level_str = "[ERROR] "; break;
        default: break;
    }
    
    printf("%s", level_str);
    vprintf(fmt, args);
    printf("\n");
    
    va_end(args);
}

// 包回调
void packet_handler(void* packet, void* user_data) {
    protocol_t* p = (protocol_t*)packet;
    uint16_t length = PROTO_NTOHS(p->length);
    printf("Received packet: cmd=%u, len=%u\n", p->cmd, length);
    
    // 处理包数据...
    printf("Data: ");
    for (int i = 0; i < length; i++) {
        putchar(p->data[i]);
    }
    printf("\n");
    
    proto_packet_free(&packet);
}

int main() {
    proto_set_logger(log_handler);
    
    proto_parser_t parser;
    proto_parser_init(&parser);
    proto_parser_set_callback(&parser, packet_handler, NULL);
    
    // 首先创建包内容
    uint8_t payload[] = {'A', 'B', 'C', 'D'};
    
    // 创建完整的包
    protocol_t* packet = proto_create_packet(0x01, 0x02, 0x03, 4, payload);
    if (!packet) {
        fprintf(stderr, "Failed to create packet\n");
        return 1;
    }
    
    // 提取包的完整数据
    size_t total_size = GET_PACKET_LEN(4);
    uint8_t sample_data[total_size];
    memcpy(sample_data, packet, total_size);
    proto_packet_free((void**)&packet);
    
    // 解析每个字节
    for (size_t i = 0; i < total_size; i++) {
        printf("Processing byte %02X\n", sample_data[i]);
        PARSE_STATUS status = proto_packet_parse(&parser, sample_data[i]);
        
        if (status == PARSE_OK) {
            printf("Packet parsed successfully\n");
        } else if (status != PARSE_INCOMPLETE) {
            printf("Parse error: %d\n", status);
            break;
        }
    }
    
    proto_parser_destroy(&parser);
    return 0;
}