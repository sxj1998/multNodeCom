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
    bus_socket_driver_t* dev = (bus_socket_driver_t*)self;
    int* fd = &(dev->bus_device.fd);
    socket_type_e type = dev->bus_device.type;

    *fd = create_socket();
    utils_assert(*fd > 0);
    
    int ret = 0;
    utils_assert(dev->bus_device.ipaddr != NULL);
    if(type == SOCKET_CLIENT_TYPE){
        int ret_addr_init = socket_client_address_init(&dev->bus_device.socket_addr, dev->bus_device.ipaddr, dev->bus_device.port);
        utils_assert(ret_addr_init == 0);
        if(ret_addr_init != 0)
            ret = -1;
    }else if(type == SOCKET_SERVER_TYPE){
        socket_server_address_init(&dev->bus_device.socket_addr, dev->bus_device.port);
        int ret_bind = socket_server_bind_socket(*fd , &dev->bus_device.socket_addr);
        utils_assert(ret_bind == 0);
        int ret_listen = socket_server_start_listening(*fd, dev->bus_device.server_max_listen_num);
        utils_assert(ret_listen == 0);
        if(ret_bind != 0 || ret_listen != 0)
            ret = -1;
    }

    return ret;
}

static int open_x(void *self)
{
    int ret = 0; 
    bus_socket_driver_t* dev = (bus_socket_driver_t*)self;
    socket_type_e type = dev->bus_device.type;
    uint8_t client_idx = dev->bus_device.server_connect_client_idx;
    int* fd = &(dev->bus_device.fd);

    if(type == SOCKET_CLIENT_TYPE){
        ret = connect_to_server(*fd, &dev->bus_device.socket_addr);
    }else if(type == SOCKET_SERVER_TYPE){
        dev->bus_device.server_connect_client_fd[client_idx] = socket_server_accept_connection(*fd, &dev->bus_device.server_connect_client_sockaddr[client_idx]);
        dev->bus_device.server_connect_client_idx++;
        TI_DEBUG("client_server_sockfd  %d , idx %d\r\n", dev->bus_device.server_connect_client_fd[client_idx], client_idx);
    }

    return ret;
}

static int close_x(void *self)
{
    bus_socket_driver_t* dev = (bus_socket_driver_t*)self;
    socket_type_e type = dev->bus_device.type;
    int* fd = &(dev->bus_device.fd);
    if(type == SOCKET_CLIENT_TYPE){
        close(*fd);
    }else if(type == SOCKET_SERVER_TYPE){
        close(*fd);
    }
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

bus_socket_driver_t* bus_socket_driver_register(const char* dev_name, uint8_t bus_id, char* socket_ip, uint16_t port, socket_type_e type)
{
    bus_socket_driver_t* socket_driver = NULL;
    socket_driver = (bus_socket_driver_t*)malloc(sizeof(bus_socket_driver_t));

    strncpy(socket_driver->bus_device.ipaddr, socket_ip, sizeof(socket_driver->bus_device.ipaddr) - 1);
    socket_driver->bus_device.ipaddr[sizeof(socket_driver->bus_device.ipaddr) - 1] = '\0'; // 确保字符串以空字符结尾
    TI_DEBUG("socket ip address %s", socket_driver->bus_device.ipaddr);

    socket_driver->bus_device.port = port;
    socket_driver->bus_device.type = type;
    socket_driver->bus_device.server_max_listen_num = SOCKET_LISTEN_MAX_NUM;
    socket_driver->bus_device.server_connect_client_idx = 0;
    
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