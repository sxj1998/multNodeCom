#include "CppUTest/TestHarness.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <termios.h>
#include <string.h>

extern "C"{
#include "serial.h"
}

int serial_fd = 0;
TEST_GROUP(test_serial_local_hard){
    void setup(){
    }

    void teardown(){
    }};

TEST(test_serial_local_hard, check_usart_init)
{
   serial_fd = open("/dev/ttyUSB0", O_RDWR | O_NOCTTY | O_NDELAY);
   CHECK_TRUE(serial_fd > 0);

   if (serial_fd < 0) {
      printf("Serial fd %d\r\n",serial_fd);
   } else {
      fcntl(serial_fd, F_SETFL, 0);
   }

   int ret = uart_setup(serial_fd);
   CHECK_EQUAL(ret, 0);
   ret = usart_deinit(serial_fd);
   CHECK_EQUAL(ret, 0);
}

TEST(test_serial_local_hard, check_usart_send_recv)
{

}

TEST(test_serial_local_hard, test2)
{

}


