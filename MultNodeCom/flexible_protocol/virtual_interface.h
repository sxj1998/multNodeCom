/* ================= virtual_interface.h ================= */
#ifndef VIRTUAL_INTERFACE_H
#define VIRTUAL_INTERFACE_H

#include "router.h"

typedef struct VirtualInterface VirtualInterface;

// 创建虚拟硬件接口
HardwareInterface* virtual_interface_create();

// 连接两个虚拟接口
void virtual_interface_connect(HardwareInterface* if1, HardwareInterface* if2);

// 销毁虚拟接口
void virtual_interface_destroy(HardwareInterface* hw_if);

#endif // VIRTUAL_INTERFACE_H