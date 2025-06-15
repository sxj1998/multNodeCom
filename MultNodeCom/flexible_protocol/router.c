/* ================= router.c ================= */
#include "router.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define ROUTING_TABLE_INIT_SIZE 8

// 节点初始化
void node_init(Node* node, uint8_t node_id) {
    memset(node, 0, sizeof(Node));
    node->node_id = node_id;
    proto_parser_init(&node->parser);
    node->routing_capacity = ROUTING_TABLE_INIT_SIZE;
    node->routing_table = malloc(sizeof(RoutingEntry) * node->routing_capacity);
}

// 节点销毁
void node_destroy(Node* node) {
    proto_parser_destroy(&node->parser);
    free(node->routing_table);
}

// 设置包处理器
void node_set_packet_handler(Node* node, void (*handler)(Node*, protocol_t*)) {
    node->packet_handler = handler;
}

// 添加路由条目
void node_add_route(Node* node, uint8_t dest_id, HardwareInterface* hw_if) {
    // 检查是否已存在
    for (int i = 0; i < node->routing_size; i++) {
        if (node->routing_table[i].dest_id == dest_id) {
            node->routing_table[i].hw_if = hw_if;
            return;
        }
    }
    
    // 扩容检查
    if (node->routing_size >= node->routing_capacity) {
        int new_capacity = node->routing_capacity * 2;
        RoutingEntry* new_table = realloc(node->routing_table, 
                                        sizeof(RoutingEntry) * new_capacity);
        if (!new_table) return;
        node->routing_table = new_table;
        node->routing_capacity = new_capacity;
    }
    
    // 添加新条目
    node->routing_table[node->routing_size++] = (RoutingEntry){
        .dest_id = dest_id,
        .hw_if = hw_if
    };
}

// 解析器回调函数
static void packet_parser_callback(void* packet_, void* user_data) {
    Node* node = (Node*)user_data;
    protocol_t* packet = (protocol_t*)packet_;
    uint16_t net_len = PROTO_NTOHS(packet->length);
    
    if (packet->dst_id == node->node_id) {
        // 目标为本节点
        printf("[NODE %d] Received packet from %d (cmd=0x%02x, len=%d)\n",
               node->node_id, packet->src_id, packet->cmd, net_len);
        
        if (node->packet_handler) {
            node->packet_handler(node, packet);
        } else {
            // proto_packet_free(packet);
        }
    } else {
        // 需要转发
        printf("[NODE %d] Forwarding packet to %d\n", node->node_id, packet->dst_id);
        node_forward_packet(node, packet);
        // proto_packet_free(packet);
    }
}

// 接收字节处理
void node_receive_byte(Node* node, uint8_t byte) {
    static bool callback_set = false;
    if (!callback_set) {
        proto_parser_set_callback(&node->parser, packet_parser_callback, node);
        callback_set = true;
    }
    proto_packet_parse(&node->parser, byte);
}

// 转发数据包
void node_forward_packet(Node* node, protocol_t* packet) {
    // 查找路由
    for (int i = 0; i < node->routing_size; i++) {
        if (node->routing_table[i].dest_id == packet->dst_id) {
            HardwareInterface* hw_if = node->routing_table[i].hw_if;
            
            // 重新打包数据（保持原始源和目标）
            uint16_t net_len = PROTO_NTOHS(packet->length);
            void* new_pkt = proto_create_packet_with_index(
                packet->src_id, 
                packet->dst_id,
                packet->cmd,
                net_len,
                packet->data,
                PROTO_NTOHS(packet->index)
            );
            
            if (new_pkt) {
                uint16_t total_len = GET_PACKET_LEN(net_len);
                hw_if->send(hw_if, (uint8_t*)new_pkt, total_len);
                proto_packet_free(&new_pkt);
            }
            return;
        }
    }
    
    printf("[NODE %d] No route to node %d\n", node->node_id, packet->dst_id);
}

// 发送数据包
bool node_send_packet(Node* node, uint8_t dest_id, uint8_t cmd, 
                     uint16_t data_len, const uint8_t* data) {
    // 查找路由
    for (int i = 0; i < node->routing_size; i++) {
        if (node->routing_table[i].dest_id == dest_id) {
            HardwareInterface* hw_if = node->routing_table[i].hw_if;
            
            // 创建数据包
            void* packet = proto_create_packet(
                node->node_id,
                dest_id,
                cmd,
                data_len,
                data
            );
            
            if (!packet) return false;
            
            // 发送数据包
            uint16_t net_len = PROTO_NTOHS(((protocol_t*)packet)->length);
            uint16_t total_len = GET_PACKET_LEN(net_len);
            bool result = hw_if->send(hw_if, (uint8_t*)packet, total_len);
            proto_packet_free(&packet);
            return result;
        }
    }
    
    printf("[NODE %d] No route to node %d\n", node->node_id, dest_id);
    return false;
}