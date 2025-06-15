#ifndef HARDWARE_INTERFACE_H
#define HARDWARE_INTERFACE_H

#include <stdint.h> // 添加标准整型头文件

// 硬件接口抽象
typedef struct HardwareInterface HardwareInterface;

// 硬件接口操作
typedef struct {
    int (*init)(HardwareInterface* hw_if);
    int (*write)(HardwareInterface* hw_if, const uint8_t* data, uint16_t len);
    int (*read)(HardwareInterface* hw_if, uint8_t* buffer, uint16_t max_len);
    void (*close)(HardwareInterface* hw_if);
} HardwareInterfaceOps;

// 硬件接口结构
struct HardwareInterface {
    void* context;          // 底层硬件上下文
    HardwareInterfaceOps ops; // 硬件操作接口
};

#endif /* HARDWARE_INTERFACE_H */