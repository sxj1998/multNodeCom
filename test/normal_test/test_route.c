/* ================= main.c ================= */
#include "router.h"
#include "virtual_interface.h"
#include <stdio.h>
#include <unistd.h>

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

int main() {
    printf("===== Multi-Hop Routing Protocol Test =====\n");
    
    // 创建网络拓扑：A -> B -> C
    printf("Creating nodes: A(1), B(2), C(3)\n");
    Node node_a, node_b, node_c;
    node_init(&node_a, 1);
    node_init(&node_b, 2);
    node_init(&node_c, 3);
    
    // 设置数据包处理器
    node_set_packet_handler(&node_a, packet_handler);
    node_set_packet_handler(&node_b, packet_handler);
    node_set_packet_handler(&node_c, packet_handler);
    
    // 创建虚拟硬件接口
    printf("Creating virtual interfaces\n");
    HardwareInterface* hw_ab = virtual_interface_create(); // A <-> B
    HardwareInterface* hw_ba = virtual_interface_create();
    HardwareInterface* hw_bc = virtual_interface_create(); // B <-> C
    HardwareInterface* hw_cb = virtual_interface_create();
    
    // 连接接口
    virtual_interface_connect(hw_ab, hw_ba);
    virtual_interface_connect(hw_bc, hw_cb);
    
    // 配置路由表
    printf("Configuring routing tables\n");
    
    // Node A: 所有流量通过 hw_ab 发送到 B
    node_add_route(&node_a, 2, hw_ab); // 到B
    node_add_route(&node_a, 3, hw_ab); // 到C（通过B）
    
    // Node B: 
    //   - 到A通过 hw_ba
    //   - 到C通过 hw_bc
    node_add_route(&node_b, 1, hw_ba);
    node_add_route(&node_b, 3, hw_bc);
    
    // Node C: 所有流量通过 hw_cb 发送到 B
    node_add_route(&node_c, 1, hw_cb); // 到A（通过B）
    node_add_route(&node_c, 2, hw_cb); // 到B
    
    // 测试1: A 直接发送给 B
    printf("\nTest 1: A -> B (Direct)\n");
    uint8_t test_data1[] = {0x01, 0x02, 0x03};
    node_send_packet(&node_a, 2, 0xA0, sizeof(test_data1), test_data1);
    
    // 测试2: A 发送给 C（需要 B 转发）
    printf("\nTest 2: A -> C (via B)\n");
    uint8_t test_data2[] = {0xAA, 0xBB, 0xCC, 0xDD};
    node_send_packet(&node_a, 3, 0xB0, sizeof(test_data2), test_data2);
    
    // 测试3: C 发送给 A（需要 B 转发）
    printf("\nTest 3: C -> A (via B)\n");
    uint8_t test_data3[] = {0x11, 0x22, 0x33, 0x44, 0x55};
    node_send_packet(&node_c, 1, 0xC0, sizeof(test_data3), test_data3);
    
    // 测试4: 发送到未知节点
    printf("\nTest 4: Send to unknown node\n");
    uint8_t test_data4[] = {0xFF};
    node_send_packet(&node_a, 4, 0xD0, sizeof(test_data4), test_data4);
    
    // 等待所有数据处理完成
    sleep(1);
    
    // 清理资源
    virtual_interface_destroy(hw_ab);
    virtual_interface_destroy(hw_ba);
    virtual_interface_destroy(hw_bc);
    virtual_interface_destroy(hw_cb);
    
    node_destroy(&node_a);
    node_destroy(&node_b);
    node_destroy(&node_c);
    
    printf("\nTest completed\n");
    return 0;
}