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
#include "serial.h"
#include "ringbuffer.h"
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

MODULE_TAG("SERICL_HARDWARE"); 

/**
 * @brief uart_setup
 * 
 * @param fd 
 * @return int 
 */
int uart_setup(int fd)
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

int serial_write(int fd, uint8_t *data, uint16_t length)
{
    int ret = write(fd, data, length);
    return ret;
}

int serial_read(int fd, uint8_t *data, uint16_t length)
{
    int ret = read(fd, data, length);
    return ret;
}
