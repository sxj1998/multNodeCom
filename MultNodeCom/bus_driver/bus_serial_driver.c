/**
 * @file bus_serial_driver.c
 * @author shexingju (1970237065@qq.com)
 * @brief 
 * @version 0.1
 * @date 2024-06-23
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#include "bus_serial_driver.h"
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

MODULE_TAG("SERICL"); 

static int init_x(void *self);
static int open_x(void *self);
static int close_x(void *self);
static int rb_read_sync_x(void *self);
static int rb_write_sync_x(void *self);
static int write_buff_x(void *self, uint8_t *buffer, uint16_t length);
static int read_buff_x(void *self, uint8_t *buffer, uint16_t length);

static const bus_interface_i bus_serial_interface = {
    .init = init_x,
    .open = open_x,
    .close = close_x,
    .sync_rx = rb_read_sync_x,
    .sync_tx = rb_write_sync_x,
    .write = write_buff_x,
    .read = read_buff_x
};

/**
 * @brief uart_setup
 * 
 * @param fd 
 * @return int 
 */
static int uart_setup(int fd)
{
    struct termios options;

    // 获取原有串口配置
    if  (tcgetattr(fd, &options) < 0) {
        return -1;
    }

    // 修改控制模式，保证程序不会占用串口
    options.c_cflag  |=  CLOCAL;

    // 修改控制模式，能够从串口读取数据
    options.c_cflag  |=  CREAD;

    // 不使用流控制
    options.c_cflag &= ~CRTSCTS;

    // 设置数据位
    options.c_cflag &= ~CSIZE;
    options.c_cflag |= CS8;

    // 设置奇偶校验位
    options.c_cflag &= ~PARENB;
    options.c_iflag &= ~INPCK; 

    // 设置停止位
    options.c_cflag &= ~CSTOPB;

    // 设置最少字符和等待时间
    options.c_cc[VMIN] = 1;     // 读数据的最小字节数
    options.c_cc[VTIME]  = 0;   //等待第1个数据，单位是10s
    
    // 修改输出模式，原始数据输出
    options.c_cflag |= CLOCAL | CREAD;
    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    options.c_oflag &= ~OPOST;
    options.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);

    // 设置波特率
    cfsetispeed(&options, B115200); 
    cfsetospeed(&options, B115200);

    // 清空终端未完成的数据
    tcflush(fd, TCIFLUSH);

    // 设置新属性
    if(tcsetattr(fd, TCSANOW, &options) < 0) {
        return -1;
    }

    return 0;
}

static int init_x(void *self)
{
    bus_serial_driver_t* dev = (bus_serial_driver_t*)self;
    int* fd = &(dev->bus_device.fd);
    int ret = 0;

    TI_DEBUG("serial init DEV_NAME %s ",dev->dev_name);
        /* 打开串口 */
    *fd = open(dev->dev_name, O_RDWR | O_NOCTTY | O_NDELAY);
    if (*fd < 0) {
        TI_DEBUG("open dev fail! ");
        ret = -1;
    } else {
        TI_DEBUG("DEV %s fd:%d ", dev->dev_name, *fd);
        fcntl(*fd, F_SETFL, 0);
    }

    /* 设置串口 */
    ret = uart_setup(*fd);
    if (ret < 0) {
        TI_DEBUG("uart setup fail! ");
        close(*fd);
        ret = -1;
    }

    return ret;
}

static int open_x(void *self)
{

    return 0;
}

static int close_x(void *self)
{
    return 0;
}

static int serial_write(void *self, uint8_t *data, uint16_t length)
{
    int ret = 0;
    bus_serial_driver_t* dev = (bus_serial_driver_t*)self;
    int fd = dev->bus_device.fd;
    ret = write(fd, data, length);

    return ret;
}

static int serial_read(void *self, uint8_t *data, uint16_t length)
{
    int ret = 0;
    bus_serial_driver_t* dev = (bus_serial_driver_t*)self;
    int fd = dev->bus_device.fd;
    ret = read(fd, data, length);

    return ret;
}

static int write_buff_x(void *self, uint8_t *data, uint16_t length)
{
    bus_serial_driver_t* dev = (bus_serial_driver_t*)self;
    struct rt_ringbuffer* write_rb = dev->bus_driver->write_rb;
    int write_rb_ret = 0;

    write_rb_ret = rt_ringbuffer_put_force(write_rb, (uint8_t *)data, (uint32_t)length);

    if(write_rb_ret > 0){
        TI_DEBUG_NO_N("serial %s write :",dev->dev_name);
        utils_buff_print(data,length);
    }
    
    return write_rb_ret;
}

static int read_buff_x(void *self, uint8_t *data, uint16_t length)
{
    bus_serial_driver_t* dev = (bus_serial_driver_t*)self;
    struct rt_ringbuffer* read_rb = dev->bus_driver->read_rb;
    int read_rb_ret = 0;

    read_rb_ret = rt_ringbuffer_get(read_rb, (uint8_t *)data, (uint32_t)length);
    
    if(read_rb_ret > 0){
        TI_DEBUG_NO_N("serial %s read:",dev->dev_name);
        utils_buff_print(data,read_rb_ret);
    }
   
    return read_rb_ret;
}


static int rb_write_sync_x(void *self)
{
    bus_serial_driver_t* dev = (bus_serial_driver_t*)self;
    struct rt_ringbuffer* write_rb = dev->bus_driver->write_rb;
    int write_serial_len = 0, write_rb_ret = 0;
    uint8_t recv_buffer[dev->bus_driver->bus_tx_buffer_size];
    memset(recv_buffer, 0, sizeof(recv_buffer));

    write_serial_len = rt_ringbuffer_get(write_rb, (uint8_t *)recv_buffer, (uint32_t)dev->bus_driver->bus_tx_buffer_size);

    if(write_serial_len > 0){
        write_rb_ret = serial_write(dev, (uint8_t *)recv_buffer, write_serial_len);
    }

    if(write_rb_ret != write_serial_len)
        return -1;

    return write_rb_ret;
}

static int rb_read_sync_x(void *self)
{
    bus_serial_driver_t* dev = (bus_serial_driver_t*)self;
    struct rt_ringbuffer* read_rb = dev->bus_driver->read_rb;
    int read_serial_len = 0, serial_read_ret = 0;
    uint8_t recv_buffer[dev->bus_driver->bus_rx_buffer_size];
    memset(recv_buffer, 0, sizeof(recv_buffer));

    read_serial_len = serial_read(dev,recv_buffer, (uint32_t)dev->bus_driver->bus_rx_buffer_size);
    
    if( read_serial_len > 0){
        serial_read_ret = rt_ringbuffer_put_force(read_rb, (uint8_t *)recv_buffer, (uint32_t)read_serial_len);
    }

    if(serial_read_ret != read_serial_len)
        return -1;

    return serial_read_ret;
}

bus_serial_driver_t* bus_serial_driver_register(const char* dev_name, uint8_t bus_id)
{
    
    bus_serial_driver_t* serial_driver = NULL;
    serial_driver = (bus_serial_driver_t*)malloc(sizeof(bus_serial_driver_t));

    strncpy(serial_driver->dev_name, dev_name, sizeof(serial_driver->dev_name) - 1);
    serial_driver->dev_name[sizeof(serial_driver->dev_name) - 1] = '\0'; // 确保字符串以空字符结尾

    int8_t ret = bus_driver_register(&serial_driver->bus_driver, &bus_serial_interface, 256, 256, "serial", SERIAL, bus_id); 
    if(ret < 0)
        goto free_serial_driver;

    return serial_driver;

free_serial_driver:
    free(serial_driver);

    return 0;
}