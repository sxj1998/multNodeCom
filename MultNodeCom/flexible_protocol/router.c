#include "router.h"
#include <stdlib.h>
#include <string.h>
#include "xlog.h"

// 解析器回调函数
static void packet_received_callback(void* packet, void* user_data);

void node_init(Node* node, uint8_t node_id, int max_interfaces) {
    if (!node) return;
    
    memset(node, 0, sizeof(Node));
    node->node_id = node_id;
    node->max_interfaces = max_interfaces;
    
    if (max_interfaces > 0) {
        node->interfaces = calloc(max_interfaces, sizeof(NodeInterface));
    }
}

void node_destroy(Node* node) {
    if (!node) return;
    
    // 释放所有接口的解析器
    for (int i = 0; i < node->interface_count; i++) {
        proto_parser_destroy(&node->interfaces[i].parser);
    }
    
    // 释放接口数组
    free(node->interfaces);
    memset(node, 0, sizeof(Node));
}

void node_set_packet_handler(Node* node, void (*handler)(Node*, protocol_t*)) {
    if (node) {
        node->packet_handler = handler;
    }
}

bool node_add_interface(Node* node, HardwareInterface* hw_if) {
    if (!node || !hw_if || node->interface_count >= node->max_interfaces) {
        return false;
    }
    
    // 初始化接口
    NodeInterface* iface = &node->interfaces[node->interface_count];
    iface->hw_if = hw_if;
    
    // 初始化协议解析器
    proto_parser_init(&iface->parser);
    proto_parser_set_callback(&iface->parser, packet_received_callback, node);
    
    node->interface_count++;
    return true;
}

PARSE_STATUS node_receive_byte(Node* node, int interface_index, uint8_t byte) {
    if (!node || interface_index < 0 || interface_index >= node->interface_count) {
        return PARSE_ERROR_INTERNAL;
    }
    
    // 将字节传递给对应接口的解析器
    return proto_packet_parse(&node->interfaces[interface_index].parser, byte);
}

void node_forward_packet(Node* node, protocol_t* packet, int exclude_interface) {
    if (!node || !packet) return;
    
    // 获取数据包长度
    uint16_t data_len = PROTO_NTOHS(packet->length);
    size_t total_len = GET_PACKET_LEN(data_len);
    
    // 遍历所有接口（排除接收接口）
    for (int i = 0; i < node->interface_count; i++) {
        if (i != exclude_interface) {
            node->interfaces[i].hw_if->ops.write(
                node->interfaces[i].hw_if, 
                (uint8_t*)packet, 
                total_len
            );
        }
    }
}

bool node_send_packet(Node* node, uint8_t dest_id, uint8_t cmd, 
                     uint16_t data_len, const uint8_t* data) {
    if (!node) return false;
    
    // 创建数据包
    void* packet = proto_create_packet(node->node_id, dest_id, cmd, data_len, data);
    if (!packet) {
        LOG_ERROR("Failed to create packet for %u", dest_id);
        return false;
    }
    
    // 获取数据包长度
    uint16_t packet_len = proto_escaped_length((protocol_t*)packet);
    
    // 发送到所有接口
    bool success = true;
    for (int i = 0; i < node->interface_count; i++) {
        int result = node->interfaces[i].hw_if->ops.write(
            node->interfaces[i].hw_if, 
            (uint8_t*)packet, 
            packet_len
        );
        
        if (result != packet_len) {
            success = false;
        }
    }
    
    // 释放数据包
    proto_packet_free(&packet);
    return success;
}

// 数据包接收回调函数
static void packet_received_callback(void* packet, void* user_data) {
    Node* node = (Node*)user_data;
    protocol_t* proto_packet = (protocol_t*)packet;
    
    // 检查目的地址
    if (proto_packet->dst_id == node->node_id) {
        // 如果是本节点，调用处理函数
        if (node->packet_handler) {
            node->packet_handler(node, proto_packet);
        } else {
            LOG_INFO("Received packet for local node, but no handler set");
            proto_packet_free(&packet);
        }
    } else {
        // 查找接收接口的索引 - 修复指针比较问题
        int receive_interface = -1;
        for (int i = 0; i < node->interface_count; i++) {
            // 比较指针值而不是指针的地址
            if (node->interfaces[i].parser.packet == proto_packet) {
                receive_interface = i;
                break;
            }
        }
        
        if (receive_interface == -1) {
            LOG_WARN("Failed to find receiving interface for packet");
            proto_packet_free(&packet);
            return;
        }
        
        // 转发数据包（排除接收接口）
        LOG_INFO("Forwarding packet for destination %u", proto_packet->dst_id);
        node_forward_packet(node, proto_packet, receive_interface);
        proto_packet_free(&packet);
    }
}