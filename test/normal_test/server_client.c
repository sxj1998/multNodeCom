#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include "protocol.h"

// 共享资源结构体
typedef struct {
    uint8_t data[1024];
    uint16_t size;
    pthread_mutex_t lock;        // 互斥锁
} shared_buffer_t;

volatile sig_atomic_t shutdown_requested = 0;  // 全局退出标志
shared_buffer_t buffer = {{0}, 0, PTHREAD_MUTEX_INITIALIZER};  // 共享缓冲区
proto_parser_t server_parser;

// ================= 信号处理函数 =================
void sigint_handler(int sig) {
    shutdown_requested = 1;
}

// ================= 服务端处理函数 =================
void server_packet_handler(protocol_t *packet, void *user_data) {
    printf("\n[Server] Received packet!\n");
    printf("Source ID: %u\n", packet->src_id);
    printf("Target ID: %u\n", packet->dst_id);
    printf("Command: %u\n", packet->cmd);
    printf("Length: %u\n", packet->length);
    
    if (packet->length > 0) {
        printf("Data: ");
        for (int i = 0; i < packet->length; i++) {
            printf("%02X ", packet->data[i]);
        }
        printf("\n");
    }
}

// ================= 服务端线程函数 =================
void *server_thread(void *arg) {
    printf("[Server] Starting...\n");
    
    // 初始化协议解析器
    proto_parser_init(&server_parser);
    proto_parser_set_callback(&server_parser, server_packet_handler, NULL);
    
    uint16_t processed = 0;  // 已处理的字节数
    
    while (!shutdown_requested) {
        // 尝试锁定共享缓冲区
        pthread_mutex_lock(&buffer.lock);
        
        if (buffer.size > processed) {
            // 处理所有未处理的数据
            while (processed < buffer.size && !shutdown_requested) {
                printf("Processing byte %d: %02x\n", processed, buffer.data[processed]);
                
                PARSE_STATUS_e status = proto_packet_parse(&server_parser, buffer.data[processed]);
                processed++;
                
                switch (status) {
                    case PARSE_HEADER_ERR:
                        printf("[Server] Header error!\n");
                        break;
                    case PARSE_LENGTH_ERR:
                        printf("[Server] Length error!\n");
                        break;
                    case PARSE_CRC_ERR:
                        printf("[Server] CRC error!\n");
                        break;
                    case PARSE_OK:
                        printf("[Server] Parse complete!\n");
                        break;
                    case PARSE_MEM_ERR:
                        printf("[Server] Memory error!\n");
                        break;
                    default:
                        break;
                }
            }
        }
        
        // 解锁共享缓冲区
        pthread_mutex_unlock(&buffer.lock);
        
        // 短暂休眠避免忙等待
        usleep(10 * 1000); // 10ms
    }
    
    // 清理工作
    proto_parser_destroy(&server_parser);
    printf("[Server] Exiting.\n");
    return NULL;
}

// ================= 客户端线程函数 =================
void *client_thread(void *arg) {
    printf("[Client] Starting...\n");
    uint8_t test_data[] = {0x01, 0x02, 0x03, 0x04, 0x05};
    
    while (!shutdown_requested) {
        // 创建要发送的测试数据
        protocol_t *packet = proto_create_packet(1, 2, 0x10, 
                                            sizeof(test_data), test_data);
        
        size_t packet_size = GET_PACKET_LEN(packet->length);
        
        // 锁定共享缓冲区
        pthread_mutex_lock(&buffer.lock);
        
        // 检查空间是否足够
        if (buffer.size + packet_size <= sizeof(buffer.data)) {
            memcpy(&buffer.data[buffer.size], packet, packet_size);
            buffer.size += packet_size;
            printf("[Client] Added packet to buffer (%zu bytes), total: %d\n", 
                   packet_size, buffer.size);
        } else {
            printf("[Client] Buffer full! Unable to add packet.\n");
        }
        
        // 解锁共享缓冲区
        pthread_mutex_unlock(&buffer.lock);
        
        // 发送间隔
        sleep(1);
        
        // 释放包内存
        proto_packet_free(&packet);
    }
    
    printf("[Client] Exiting.\n");
    return NULL;
}

// ================= 主函数 =================
int main() {
    pthread_t s_tid, c_tid;
    
    // 注册信号处理
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = sigint_handler;
    sigaction(SIGINT, &sa, NULL);
    
    printf("Press Ctrl+C to exit...\n");
    
    // 创建服务端线程
    if (pthread_create(&s_tid, NULL, server_thread, NULL)) {
        perror("pthread_create server");
        exit(EXIT_FAILURE);
    }
    
    // 创建客户端线程
    if (pthread_create(&c_tid, NULL, client_thread, NULL)) {
        perror("pthread_create client");
        exit(EXIT_FAILURE);
    }
    
    // 等待线程结束
    pthread_join(s_tid, NULL);
    pthread_join(c_tid, NULL);
    
    // 销毁互斥锁
    pthread_mutex_destroy(&buffer.lock);
    
    printf("Main: Program exited cleanly.\n");
    return 0;
}