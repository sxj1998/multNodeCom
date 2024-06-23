/**
 * @file serial.h
 * @author shexingju (1970237065@qq.com)
 * @brief 
 * @version 0.1
 * @date 2024-06-23
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#ifndef __SERIAL_H__
#define __SERIAL_H__

#include "stdint.h"

#ifdef __cplusplus
extern "C" {
#endif

int uart_setup(int fd);

int usart_deinit(int fd);

int serial_write(int fd, uint8_t *data, uint16_t length);

int serial_read(int fd, uint8_t *data, uint16_t length);

#ifdef __cplusplus
}
#endif

#endif /* __SERIAL_H__ */



