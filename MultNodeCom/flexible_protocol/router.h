/* ================= router.h ================= */
#ifndef ROUTER_H
#define ROUTER_H

#include "protocol.h"
#include "hardware_interface.h"
#include <stdbool.h>

// 节点接口结构
typedef struct NodeInterface {
    HardwareInterface* hw_if;   // 硬件接口
    proto_parser_t parser;      // 协议解析器
} NodeInterface; 

// 节点结构
typedef struct Node {
    uint8_t node_id;                    // 本节点ID
    void (*packet_handler)(struct Node*, protocol_t*);  // 数据包处理函数
    NodeInterface* interfaces;          // 接口数组
    int interface_count;                // 当前接口数量
    int max_interfaces;                 // 最大接口数量
} Node;

// 节点接口函数
void node_init(Node* node, uint8_t node_id, int max_interfaces);
void node_destroy(Node* node);
void node_set_packet_handler(Node* node, void (*handler)(Node*, protocol_t*));
bool node_add_interface(Node* node, HardwareInterface* hw_if);
PARSE_STATUS node_receive_byte(Node* node, int interface_index, uint8_t byte);
void node_forward_packet(Node* node, protocol_t* packet, int exclude_interface);
bool node_send_packet(Node* node, uint8_t dest_id, uint8_t cmd, 
                     uint16_t data_len, const uint8_t* data);

#endif // ROUTER_H