/* router.h */
#ifndef ROUTER_H
#define ROUTER_H

#include "protocol.h"
#include "hardware_interface.h"
#include <stdbool.h>

// 路由表条目
typedef struct {
    uint8_t dest_id;
    HardwareInterface* hw_if;
} RoutingEntry;

// 节点结构
typedef struct Node {
    uint8_t node_id;
    proto_parser_t parser;
    void (*packet_handler)(struct Node*, protocol_t*);
    
    RoutingEntry* routing_table;
    int routing_size;
    int routing_capacity;
    
    HardwareInterface* hw_if; // 关联的硬件接口
} Node;

// 节点接口
void node_init(Node* node, uint8_t node_id, HardwareInterface* hw_if);
void node_destroy(Node* node);
void node_set_packet_handler(Node* node, void (*handler)(Node*, protocol_t*));
void node_add_route(Node* node, uint8_t dest_id, HardwareInterface* hw_if);
PARSE_STATUS node_receive_byte(Node* node, uint8_t byte);
void node_forward_packet(Node* node, protocol_t* packet);
bool node_send_packet(Node* node, uint8_t dest_id, uint8_t cmd, 
                     uint16_t data_len, const uint8_t* data);

#endif // ROUTER_H