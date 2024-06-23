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
TEST_GROUP(cppUtest_serial_local_hard_test){
    void setup(){
         serial_fd = open("/dev/ttyUSB0", O_RDWR | O_NOCTTY | O_NDELAY);
         CHECK_TRUE(serial_fd > 0);
         fcntl(serial_fd, F_SETFL, 0);
         int ret = uart_setup(serial_fd);
         CHECK_EQUAL(ret, 0);
    }

    void teardown(){
         int ret = usart_deinit(serial_fd);
         CHECK_EQUAL(ret, 0);
    }
};

#define TEST_BYTE_NUM  16
TEST(cppUtest_serial_local_hard_test, check_usart_send_recv)
{
   uint8_t test_buffer_send[TEST_BYTE_NUM] = { 0 };
   uint8_t test_buffer_recv[TEST_BYTE_NUM] = { 0 };
   for(int i=0; i<TEST_BYTE_NUM; i++)
      test_buffer_send[i] = i;
   
   int ret = serial_write(serial_fd,test_buffer_send,TEST_BYTE_NUM);
   CHECK_EQUAL(ret, TEST_BYTE_NUM);

   ret = serial_read(serial_fd,test_buffer_recv,TEST_BYTE_NUM);
   CHECK_EQUAL(ret, TEST_BYTE_NUM);

   for(int i=0; i<TEST_BYTE_NUM; i++)
      CHECK_EQUAL(test_buffer_send[i], test_buffer_recv[i]);
}




