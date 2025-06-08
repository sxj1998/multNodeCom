#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include "protocol.h"

uint8_t copy_data[1024] = {};
uint16_t data_index = 0;
// 定义管道用于线程间通信
proto_parser_t server_parser;

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
    usleep(100*1000);
    printf("[Server] Starting...\n");
    static uint16_t i = 0;
    // 初始化协议解析器
    proto_parser_init(&server_parser);
    proto_parser_set_callback(&server_parser, server_packet_handler, NULL);
    while(1){
        if(data_index > 0){
            printf("handle data: ");
            while (data_index-- > 0) {
                printf("%02x ", copy_data[i]);
                PARSE_STATUS_e status = proto_packet_parse(&server_parser, copy_data[i++]);
                
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
                        // 继续解析
                        break;
                }
            }
            i = 0;
            data_index = 0;
            usleep(100*1000);
        }
    }
    proto_parser_destroy(&server_parser);
    printf("[Server] Exiting.\n");
    return NULL;
}

// ================= 客户端处理函数 =================
void client_packet_handler(protocol_t *packet, void *user_data) {
    printf("\n[Client] Received response packet!\n");
    printf("Source ID: %u\n", packet->src_id);
    printf("Command: %u\n", packet->cmd);
    printf("Data length: %u\n", packet->length);
    
    // 在这里处理服务器响应...
}

// ================= 客户端线程函数 =================
void *client_thread(void *arg) {
    printf("[Client] Starting...\n");
    uint8_t test_data[] = {0x01, 0x02, 0x03, 0x04, 0x05};
    while(1){
        // 创建要发送的测试数据
        
        protocol_t *packet = proto_create_packet(1, 2, 0x10, 
                                            sizeof(test_data), test_data);
        
        size_t packet_size = GET_PACKET_LEN(packet->length);
        data_index = packet_size;
        memcpy(copy_data, packet, packet_size);
        printf("[Client] Sending %zu bytes...\n", packet_size);
        for(int i=0; i<packet_size; i++)
            printf("%02x ", copy_data[i]);
        printf("\r\n");
        proto_packet_free(&packet);

        sleep(1);
    }

    return NULL;
}

// ================= 主函数 =================
int main() {
    pthread_t s_tid, c_tid;
    
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
    
    return 0;
}