/* ================= virtual_interface.c ================= */
#include "virtual_interface.h"
#include <stdlib.h>

typedef struct VirtualInterface {
    void (*receive_cb)(HardwareInterface* hw_if, const uint8_t* data, uint16_t len);
    HardwareInterface* peer; // 连接的对端接口
} VirtualInterface;

// 虚拟接口发送函数
static bool virtual_send(HardwareInterface* hw_if, const uint8_t* data, uint16_t len) {
    VirtualInterface* vif = (VirtualInterface*)hw_if->user_data;
    if (vif->peer && vif->receive_cb) {
        // 模拟传输延迟
        for (uint16_t i = 0; i < len; i++) {
            vif->receive_cb(vif->peer, &data[i], 1);
        }
        return true;
    }
    return false;
}

// 设置接收回调
static void virtual_set_receive_cb(HardwareInterface* hw_if, 
                                  void (*cb)(HardwareInterface* hw_if, const uint8_t* data, uint16_t len)) {
    VirtualInterface* vif = (VirtualInterface*)hw_if->user_data;
    vif->receive_cb = cb;
}

// 创建虚拟硬件接口
HardwareInterface* virtual_interface_create() {
    VirtualInterface* vif = malloc(sizeof(VirtualInterface));
    vif->receive_cb = NULL;
    vif->peer = NULL;
    
    HardwareInterface* hw_if = malloc(sizeof(HardwareInterface));
    hw_if->send = virtual_send;
    hw_if->set_receive_cb = virtual_set_receive_cb;
    hw_if->user_data = vif;
    
    return hw_if;
}

// 连接两个虚拟接口
void virtual_interface_connect(HardwareInterface* if1, HardwareInterface* if2) {
    VirtualInterface* vif1 = (VirtualInterface*)if1->user_data;
    VirtualInterface* vif2 = (VirtualInterface*)if2->user_data;
    
    vif1->peer = if2;
    vif2->peer = if1;
}

// 销毁虚拟接口
void virtual_interface_destroy(HardwareInterface* hw_if) {
    if (!hw_if) return;
    
    VirtualInterface* vif = (VirtualInterface*)hw_if->user_data;
    free(vif);
    free(hw_if);
}