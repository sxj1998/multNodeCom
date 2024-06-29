/**
 * @file bus_socket_driver.c
 * @author shexingju (1970237065@qq.com)
 * @brief 
 * @version 0.1
 * @date 2024-06-23
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#include "socket.h"
#include "bus_socket_driver.h"
#include "bus_driver.h"
#include "ringbuffer.h"
#include "bus_type.h"
#include "utilsPrintf.h"
#include "utilsAssert.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <termios.h>
#include <string.h>

MODULE_TAG("SOCKET"); 

static int init_x(void *self);
static int open_x(void *self);
static int close_x(void *self);
static int rb_read_sync_x(void *self);
static int rb_write_sync_x(void *self);
static int write_buff_x(void *self, uint8_t *buffer, uint16_t length);
static int read_buff_x(void *self, uint8_t *buffer, uint16_t length);

static const bus_interface_i bus_socket_interface = {
    .init = init_x,
    .open = open_x,
    .close = close_x,
    .sync_rx = rb_read_sync_x,
    .sync_tx = rb_write_sync_x,
    .write = write_buff_x,
    .read = read_buff_x
};

static int init_x(void *self)
{
    // bus_socket_driver_t* dev = (bus_socket_driver_t*)self;
    // int* fd = &(dev->bus_device.fd);
    // int ret = 0;

    // TI_DEBUG("socket init DEV_NAME %s ",dev->dev_name);
    //     /* 打开串口 */
    // *fd = open(dev->dev_name, O_RDWR | O_NOCTTY | O_NDELAY);
    // if (*fd < 0) {
    //     TI_DEBUG("open dev fail! ");
    //     ret = -1;
    // } else {
    //     TI_DEBUG("DEV %s fd:%d ", dev->dev_name, *fd);
    //     fcntl(*fd, F_SETFL, 0);
    // }

    // /* 设置串口 */
    // ret = uart_setup(*fd);
    // if (ret < 0) {
    //     TI_DEBUG("uart setup fail! ");
    //     close(*fd);
    //     ret = -1;
    // }

    // return ret;

    return 0;
}

static int open_x(void *self)
{

    return 0;
}

static int close_x(void *self)
{
    bus_socket_driver_t* dev = (bus_socket_driver_t*)self;

    return 0;
}

static int write_buff_x(void *self, uint8_t *data, uint16_t length)
{
    bus_socket_driver_t* dev = (bus_socket_driver_t*)self;
    struct rt_ringbuffer* write_rb = dev->bus_driver->write_rb;
    int write_rb_ret = 0;

    write_rb_ret = rt_ringbuffer_put_force(write_rb, (uint8_t *)data, (uint32_t)length);

    if(write_rb_ret > 0){
        TI_DEBUG_NO_N("socket %s write :",dev->dev_name);
        utils_buff_print(data,length);
    }
    
    return write_rb_ret;
}

static int read_buff_x(void *self, uint8_t *data, uint16_t length)
{
    bus_socket_driver_t* dev = (bus_socket_driver_t*)self;
    struct rt_ringbuffer* read_rb = dev->bus_driver->read_rb;
    int read_rb_ret = 0;

    read_rb_ret = rt_ringbuffer_get(read_rb, (uint8_t *)data, (uint32_t)length);
    
    if(read_rb_ret > 0){
        TI_DEBUG_NO_N("socket %s read:",dev->dev_name);
        utils_buff_print(data,read_rb_ret);
    }
   
    return read_rb_ret;
}


static int rb_write_sync_x(void *self)
{
    bus_socket_driver_t* dev = (bus_socket_driver_t*)self;
    struct rt_ringbuffer* write_rb = dev->bus_driver->write_rb;
    int write_socket_len = 0, write_rb_ret = 0;
    uint8_t recv_buffer[dev->bus_driver->bus_tx_buffer_size];
    memset(recv_buffer, 0, sizeof(recv_buffer));

    write_socket_len = rt_ringbuffer_get(write_rb, (uint8_t *)recv_buffer, (uint32_t)dev->bus_driver->bus_tx_buffer_size);

    if(write_socket_len > 0){
        write_rb_ret = socket_write(dev->bus_device.fd, (uint8_t *)recv_buffer, write_socket_len);
    }

    if(write_rb_ret != write_socket_len)
        return -1;

    return write_rb_ret;
}

static int rb_read_sync_x(void *self)
{
    bus_socket_driver_t* dev = (bus_socket_driver_t*)self;
    struct rt_ringbuffer* read_rb = dev->bus_driver->read_rb;
    int read_socket_len = 0, socket_read_ret = 0;
    uint8_t recv_buffer[dev->bus_driver->bus_rx_buffer_size];
    memset(recv_buffer, 0, sizeof(recv_buffer));

    read_socket_len = socket_read(dev->bus_device.fd,recv_buffer, (uint32_t)dev->bus_driver->bus_rx_buffer_size);
    
    if( read_socket_len > 0){
        socket_read_ret = rt_ringbuffer_put_force(read_rb, (uint8_t *)recv_buffer, (uint32_t)read_socket_len);
    }

    if(socket_read_ret != read_socket_len)
        return -1;

    return socket_read_ret;
}

bus_socket_driver_t* bus_socket_driver_register(const char* dev_name, uint8_t bus_id)
{
    
    bus_socket_driver_t* socket_driver = NULL;
    socket_driver = (bus_socket_driver_t*)malloc(sizeof(bus_socket_driver_t));

    strncpy(socket_driver->dev_name, dev_name, sizeof(socket_driver->dev_name) - 1);
    socket_driver->dev_name[sizeof(socket_driver->dev_name) - 1] = '\0'; // 确保字符串以空字符结尾

    int8_t ret = bus_driver_register(&socket_driver->bus_driver, &bus_socket_interface, 256, 256, "socket", SOCKET, bus_id); 
    if(ret < 0)
        goto free_socket_driver;

    return socket_driver;

free_socket_driver:
    free(socket_driver);

    return 0;
}