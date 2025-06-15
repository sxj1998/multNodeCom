/* ================= router.h ================= */
#ifndef ROUTER_H
#define ROUTER_H

#include "protocol.h"
#include <stdbool.h>

// 节点类型定义
typedef struct Node Node;
typedef struct HardwareInterface HardwareInterface;

// 路由表条目
typedef struct {
    uint8_t dest_id;
    HardwareInterface* hw_if;
} RoutingEntry;

// 节点结构
struct Node {
    uint8_t node_id;
    proto_parser_t parser;
    void (*packet_handler)(Node*, protocol_t*);
    
    RoutingEntry* routing_table;
    int routing_size;
    int routing_capacity;
};

// 硬件接口抽象
struct HardwareInterface {
    bool (*send)(HardwareInterface* hw_if, const uint8_t* data, uint16_t len);
    void (*set_receive_cb)(HardwareInterface* hw_if, 
                          void (*cb)(HardwareInterface* hw_if, const uint8_t* data, uint16_t len));
    void* user_data;  // 硬件特定数据
};

// 路由器接口
void node_init(Node* node, uint8_t node_id);
void node_destroy(Node* node);
void node_set_packet_handler(Node* node, void (*handler)(Node*, protocol_t*));
void node_add_route(Node* node, uint8_t dest_id, HardwareInterface* hw_if);
void node_receive_byte(Node* node, uint8_t byte);
void node_forward_packet(Node* node, protocol_t* packet);
bool node_send_packet(Node* node, uint8_t dest_id, uint8_t cmd, 
                     uint16_t data_len, const uint8_t* data);

#endif // ROUTER_H