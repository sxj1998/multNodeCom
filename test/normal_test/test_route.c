/* main.c */
#include "router.h"
#include "socket_interface.h"
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <sys/select.h>

#define MAX_NODES 3

// 数据包处理回调
void packet_handler(Node* node, protocol_t* packet) {
    uint16_t net_len = PROTO_NTOHS(packet->length);
    printf("[APP %d] Received packet from %d: cmd=0x%02x, len=%d\n",
           node->node_id, packet->src_id, packet->cmd, net_len);
    
    if (net_len > 0) {
        printf("Data: ");
        for (int i = 0; i < net_len; i++) {
            printf("%02X ", packet->data[i]);
        }
        printf("\n");
    }
    
    proto_packet_free((void**)&packet);
}

// 节点运行函数
void* node_run(void* arg) {
    Node* node = (Node*)arg;
    printf("Node %d started\n", node->node_id);
    
    // 初始化硬件接口
    if (node->hw_if && node->hw_if->ops.init) {
        node->hw_if->ops.init(node->hw_if);
    }
    
    return NULL;
}

// 处理节点接收
void process_node_receive(Node* node) {
    if (!node->hw_if) return;
    
    uint8_t buffer[1024];
    int n = node->hw_if->ops.read(node->hw_if, buffer, sizeof(buffer));
    if (n > 0) {
        for (int i = 0; i < n; i++) {
            node_receive_byte(node, buffer[i]);
        }
    }
}

int main() {
    printf("===== Multi-Hop Routing Protocol Test =====\n");
    
    // 创建节点
    Node nodes[MAX_NODES];
    pthread_t threads[MAX_NODES];
    
    // 创建硬件接口
    HardwareInterface* hw_a = socket_interface_create(NULL, 8000, true); // 节点A作为服务器
    HardwareInterface* hw_b_client = socket_interface_create("127.0.0.1", 8000, false); // B连接A
    HardwareInterface* hw_b_server = socket_interface_create(NULL, 8001, true); // B作为服务器
    HardwareInterface* hw_c = socket_interface_create("127.0.0.1", 8001, false); // C连接B
    
    // 初始化节点
    node_init(&nodes[0], 1, hw_a);      // Node A
    node_init(&nodes[1], 2, hw_b_client); // Node B (client to A)
    node_init(&nodes[2], 3, hw_c);      // Node C
    
    // 设置数据包处理器
    for (int i = 0; i < MAX_NODES; i++) {
        node_set_packet_handler(&nodes[i], packet_handler);
    }
    
    // 配置路由表
    printf("Configuring routing tables\n");
    
    // Node A: 所有流量通过 hw_a 发送到 B
    node_add_route(&nodes[0], 2, hw_a); // 到B
    node_add_route(&nodes[0], 3, hw_a); // 到C（通过B）
    
    // Node B: 
    //   - 到A通过 hw_b_client
    //   - 到C通过 hw_b_server
    node_add_route(&nodes[1], 1, hw_b_client);
    node_add_route(&nodes[1], 3, hw_b_server);
    
    // Node C: 所有流量通过 hw_c 发送到 B
    node_add_route(&nodes[2], 1, hw_c); // 到A（通过B）
    node_add_route(&nodes[2], 2, hw_c); // 到B
    
    // 启动节点线程
    for (int i = 0; i < MAX_NODES; i++) {
        pthread_create(&threads[i], NULL, node_run, &nodes[i]);
    }
    
    // 等待节点初始化
    sleep(2);
    
    // 主循环使用select处理I/O
    while (1) {
        fd_set read_fds;
        int max_fd = 0;
        
        FD_ZERO(&read_fds);
        
        // 添加所有socket到fd_set
        for (int i = 0; i < MAX_NODES; i++) {
            if (nodes[i].hw_if) {
                int fd = socket_interface_get_fd(nodes[i].hw_if);
                if (fd > 0) {
                    FD_SET(fd, &read_fds);
                    if (fd > max_fd) max_fd = fd;
                }
            }
        }
        
        // 设置超时为1秒
        struct timeval timeout;
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;
        
        int activity = select(max_fd + 1, &read_fds, NULL, NULL, &timeout);
        
        if (activity > 0) {
            // 处理可读socket
            for (int i = 0; i < MAX_NODES; i++) {
                if (nodes[i].hw_if) {
                    int fd = socket_interface_get_fd(nodes[i].hw_if);
                    if (fd > 0 && FD_ISSET(fd, &read_fds)) {
                        process_node_receive(&nodes[i]);
                    }
                }
            }
        }
        
        // 测试1: A 发送给 B
        static bool test1_done = false;
        if (!test1_done) {
            printf("\nTest 1: A -> B (Direct)\n");
            uint8_t test_data1[] = {0x01, 0x02, 0x03};
            node_send_packet(&nodes[0], 2, 0xA0, sizeof(test_data1), test_data1);
            test1_done = true;
        }
        
        // 测试2: A 发送给 C（需要 B 转发）
        static bool test2_done = false;
        if (test1_done && !test2_done) {
            printf("\nTest 2: A -> C (via B)\n");
            uint8_t test_data2[] = {0xAA, 0xBB, 0xCC, 0xDD};
            node_send_packet(&nodes[0], 3, 0xB0, sizeof(test_data2), test_data2);
            test2_done = true;
        }
        
        // 测试3: C 发送给 A（需要 B 转发）
        static bool test3_done = false;
        if (test2_done && !test3_done) {
            printf("\nTest 3: C -> A (via B)\n");
            uint8_t test_data3[] = {0x11, 0x22, 0x33, 0x44, 0x55};
            node_send_packet(&nodes[2], 1, 0xC0, sizeof(test_data3), test_data3);
            test3_done = true;
        }
        
        // 测试4: 发送到未知节点
        static bool test4_done = false;
        if (test3_done && !test4_done) {
            printf("\nTest 4: Send to unknown node\n");
            uint8_t test_data4[] = {0xFF};
            node_send_packet(&nodes[0], 4, 0xD0, sizeof(test_data4), test_data4);
            test4_done = true;
        }
        
        // 所有测试完成后退出
        if (test4_done) {
            sleep(1); // 等待最后的数据处理
            break;
        }
    }
    
    // 清理资源
    for (int i = 0; i < MAX_NODES; i++) {
        node_destroy(&nodes[i]);
    }
    
    socket_interface_destroy(hw_a);
    socket_interface_destroy(hw_b_client);
    socket_interface_destroy(hw_b_server);
    socket_interface_destroy(hw_c);
    
    printf("\nTest completed\n");
    return 0;
}