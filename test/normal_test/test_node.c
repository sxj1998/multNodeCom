#include "router.h"
#include "socket_interface.h"
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PORT1 8888
#define PORT2 8889
#define NODE1_ID 1
#define NODE2_ID 2

void node1_packet_handler(Node* node, protocol_t* packet) {
    uint16_t len = PROTO_NTOHS(packet->length);
    printf("\n[NODE1] HANDLER: Received packet from %d: cmd=0x%02X, len=%d\n", 
           packet->src_id, packet->cmd, len);
    
    if (len > 0) {
        printf("Data: ");
        for (int i = 0; i < len; i++) {
            printf("%02X ", packet->data[i]);
        }
        printf("\n");
    }
    
    proto_packet_free((void**)&packet);
    fflush(stdout);
}

void node2_packet_handler(Node* node, protocol_t* packet) {
    uint16_t len = PROTO_NTOHS(packet->length);
    printf("\n[NODE2] HANDLER: Received packet from %d: cmd=0x%02X, len=%d\n", 
           packet->src_id, packet->cmd, len);
    
    if (len > 0) {
        printf("Data: ");
        for (int i = 0; i < len; i++) {
            printf("%02X ", packet->data[i]);
        }
        printf("\n");
    }
    
    // 回发响应
    uint8_t response[] = {0xAA, 0xBB, 0xCC};
    printf("[NODE2] Sending response to %d\n", packet->src_id);
    node_send_packet(node, packet->src_id, 0x80, sizeof(response), response);
    
    proto_packet_free((void**)&packet);
    fflush(stdout);
}

// 增强解析器回调日志
static void packet_parser_callback(void* packet_, void* user_data) {
    Node* node = (Node*)user_data;
    protocol_t* packet = (protocol_t*)packet_;
    uint16_t net_len = PROTO_NTOHS(packet->length);
    
    printf("\n[NODE %d] PARSER CALLBACK: packet from %d to %d, cmd=0x%02x, len=%d\n",
           node->node_id, packet->src_id, packet->dst_id, packet->cmd, net_len);
    
    if (packet->dst_id == node->node_id) {
        // 目标为本节点
        printf("[NODE %d] Handling packet locally\n", node->node_id);
        
        if (node->packet_handler) {
            node->packet_handler(node, packet);
        } else {
            proto_packet_free((void**)&packet);
        }
    } else {
        // 需要转发
        printf("[NODE %d] Forwarding packet to %d\n", node->node_id, packet->dst_id);
        node_forward_packet(node, packet);
        proto_packet_free((void**)&packet);
    }
    fflush(stdout);
}

void* node_thread(void* arg) {
    int node_id = ((int*)arg)[0];
    int port = ((int*)arg)[1];
    int peer_port = ((int*)arg)[2];
    
    printf("\n[NODE %d] STARTING: Creating server on port %d\n", node_id, port);
    HardwareInterface* hw_if = socket_server_create(port);
    if (!hw_if) {
        printf("[NODE %d] ERROR: Server creation failed\n", node_id);
        return NULL;
    }
    printf("[NODE %d] Server ready on port %d\n", node_id, port);
    
    Node node;
    node_init(&node, node_id, hw_if);
    proto_parser_set_callback(&node.parser, packet_parser_callback, &node);
    node_set_packet_handler(&node, (node_id == NODE1_ID) ? node1_packet_handler : node2_packet_handler);
    
    // 等待对方节点启动
    printf("[NODE %d] Waiting 2s for peer node to start...\n", node_id);
    sleep(2);
    
    // 添加路由到对方节点
    printf("[NODE %d] Adding route to peer node on port %d\n", node_id, peer_port);
    HardwareInterface* route_to_peer = socket_client_create("127.0.0.1", peer_port);
    if (!route_to_peer) {
        printf("[NODE %d] ERROR: Failed to create route to peer\n", node_id);
        node_destroy(&node);
        socket_interface_destroy(&hw_if);
        return NULL;
    }
    node_add_route(&node, (node_id == NODE1_ID) ? NODE2_ID : NODE1_ID, route_to_peer);
    
    printf("[NODE %d] READY: Node started and connected\n", node_id);
    fflush(stdout);
    
    uint8_t byte;
    while (1) {
        int ret = hw_if->ops.read(hw_if, &byte, 1);
        if (ret > 0) {
            // 修复：直接调用函数，不获取返回值
            node_receive_byte(&node, byte);
        } else if (ret < 0) {
            perror("Node read error");
            break;
        } else {
            usleep(1000);
        }
    }
    
    node_destroy(&node);
    socket_interface_destroy(&hw_if);
    socket_interface_destroy(&route_to_peer);
    printf("[NODE %d] EXITING\n", node_id);
    return NULL;
}

void* test_sender_thread(void* arg) {
    printf("\n[TEST] STARTING: Waiting 4 seconds for nodes to start...\n");
    sleep(4);
    
    printf("[TEST] Creating sender interface to Node1\n");
    HardwareInterface* to_node1 = socket_client_create("127.0.0.1", PORT1);
    if (!to_node1) {
        printf("[TEST] ERROR: Failed to create sender interface\n");
        return NULL;
    }
    printf("[TEST] Connected to Node1\n");
    
    uint8_t test_data[] = {0x11, 0x22, 0x33, 0x44};
    printf("[TEST] Creating test packet to Node2\n");
    void* packet = proto_create_packet(NODE1_ID, NODE2_ID, 0x01, 
                                     sizeof(test_data), test_data);
    if (!packet) {
        printf("[TEST] ERROR: Failed to create packet\n");
        socket_interface_destroy(&to_node1);
        return NULL;
    }
    
    uint16_t len = GET_PACKET_LEN(sizeof(test_data));
    printf("[TEST] Sending test packet (%d bytes) to Node1\n", len);
    
    // 打印数据包内容
    printf("Packet content: ");
    for (int i = 0; i < len; i++) {
        printf("%02X ", ((uint8_t*)packet)[i]);
    }
    printf("\n");
    
    to_node1->ops.write(to_node1, (uint8_t*)packet, len);
    proto_packet_free(&packet);
    
    printf("[TEST] Sent test packet to Node1\n");
    
    // 等待响应
    printf("[TEST] Waiting 5s for response...\n");
    sleep(5);
    
    socket_interface_destroy(&to_node1);
    printf("[TEST] EXITING\n");
    return NULL;
}

int main() {
    pthread_t n1_tid, n2_tid, test_tid;
    
    // 节点配置参数 [node_id, port, peer_port]
    int node1_args[3] = {NODE1_ID, PORT1, PORT2};
    int node2_args[3] = {NODE2_ID, PORT2, PORT1};
    
    printf("================ NODE TEST ================\n");
    
    printf("\nStarting Node1 thread...\n");
    pthread_create(&n1_tid, NULL, node_thread, node1_args);
    
    printf("\nStarting Node2 thread...\n");
    pthread_create(&n2_tid, NULL, node_thread, node2_args);
    
    printf("\nStarting test sender thread...\n");
    pthread_create(&test_tid, NULL, test_sender_thread, NULL);
    
    pthread_join(n1_tid, NULL);
    pthread_join(n2_tid, NULL);
    pthread_join(test_tid, NULL);
    
    printf("\n================ TEST COMPLETED ================\n");
    return 0;
}