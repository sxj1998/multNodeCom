#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "protocol.h"
#include "utils/crc16.h"
#include "xlog.h"

/* 自定义日志处理器实现 */
void custom_log_handler(LogLevel level, const char* message, void* user_data) {
    // 从用户数据获取日志文件名
    const char* filename = (const char*)user_data;
    
    // 仅记录警告及以上级别的日志到文件
    if (level >= LOG_LEVEL_WARNING && filename != NULL) {
        FILE* logfile = fopen(filename, "a");
        if (logfile) {
            fprintf(logfile, "%s", message);
            fclose(logfile);
        }
    }
    
    // 所有日志都输出到控制台 (带颜色)
    switch (level) {
        case LOG_LEVEL_DEBUG:
            printf("\033[0;36m"); // 青色
            break;
        case LOG_LEVEL_INFO:
            printf("\033[0;32m"); // 绿色
            break;
        case LOG_LEVEL_WARNING:
            printf("\033[0;33m"); // 黄色
            break;
        case LOG_LEVEL_ERROR:
            printf("\033[0;31m"); // 红色
            break;
        case LOG_LEVEL_CRITICAL:
            printf("\033[1;31m"); // 粗体红色
            break;
        default:
            printf("\033[0m"); // 默认
    }
    printf("%s\033[0m", message); // 重置颜色
}

// 包回调
void packet_handler(void* packet, void* user_data) {
    protocol_t* p = (protocol_t*)packet;
    uint16_t length = PROTO_NTOHS(p->length);
    printf("Received packet:index=%u cmd=%u, len=%u\n",p->index, p->cmd, length);
    
    // 处理包数据...
    printf("Data: ");
    for (int i = 0; i < length; i++) {
        putchar(p->data[i]);
    }
    printf("\n");
    
    proto_packet_free(&packet);
}

int main() {    

    // 初始化日志系统
    log_init(LOG_LEVEL_DEBUG, 
            0, 
            custom_log_handler, NULL);
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
        LOG_INFO("Processing byte %02X\n", sample_data[i]);
        PARSE_STATUS status = proto_packet_parse(&parser, sample_data[i]);
        
        if (status == PARSE_OK) {
            LOG_INFO("Packet parsed successfully\n");
        } else if (status != PARSE_INCOMPLETE) {
            LOG_INFO("Parse error: %d\n", status);
            break;
        }
    }
    
    proto_parser_destroy(&parser);
    return 0;
}