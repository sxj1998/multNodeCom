#include "CppUTest/TestHarness.h"

#include "bus_serial_driver.h"
#include "utilsPrintf.h"
#include "utilsAssert.h"

#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h> 
#include <assert.h>
#include <fcntl.h>
#include <termios.h>

pthread_t thread_recv_sync_id, thread_write_sync_id, thread_recv_id;

void* thread_recv_sync(void* arg)
{
    bus_serial_driver_t* serial = (bus_serial_driver_t*)arg;
    while (1)
    {
        bus_sync_rx((bus_driver_t**)serial);
        usleep(1*1000);
    }
}

void* thread_send_sync(void* arg)
{
    bus_serial_driver_t* serial = (bus_serial_driver_t*)arg;
    while (1)
    {
        bus_sync_tx((bus_driver_t**)serial);
        usleep(1*1000);
    }
}

#define  USART_DEV    "/dev/ttyUSB0"
#define  BUS_ID       1
#define  TEST_BUFFER_NUM  255
uint8_t test_send_buff[TEST_BUFFER_NUM] = {0};
uint8_t test_recv_buff[TEST_BUFFER_NUM] = {0};
bus_serial_driver_t* serial = NULL;

TEST_GROUP(cppUtest_serial_local_rb){
    void setup(){
      for(int i=0; i<TEST_BUFFER_NUM; i++)
      {
         test_send_buff[i] = i;
      }

      serial = bus_serial_driver_register(USART_DEV, BUS_ID);
      CHECK_TRUE(serial != NULL);
      CHECK_EQUAL(serial->bus_driver->bus_id, BUS_ID);
      STRCMP_EQUAL(serial->dev_name, USART_DEV);
      int ret = bus_init((bus_driver_t**)serial);
      CHECK_EQUAL(ret, 0);

      pthread_create(&thread_recv_sync_id,NULL,thread_recv_sync, serial);
      pthread_detach(thread_recv_sync_id);
      pthread_create(&thread_write_sync_id,NULL,thread_send_sync, serial);
      pthread_detach(thread_write_sync_id);

    }

    void teardown(){
         int ret = bus_close((bus_driver_t**)serial);
         CHECK_EQUAL(ret, 0);
    }
};


TEST(cppUtest_serial_local_rb, check_usart_send_recv)
{
   int ret = bus_write((bus_driver_t**)serial,test_send_buff,TEST_BUFFER_NUM);
   CHECK_EQUAL(ret, TEST_BUFFER_NUM);

   usleep(1000*100);

   ret = bus_read((bus_driver_t**)serial, test_recv_buff, TEST_BUFFER_NUM); 
   CHECK_EQUAL(ret, TEST_BUFFER_NUM);

   for(int i=0; i<TEST_BUFFER_NUM; i++)
      CHECK_EQUAL(test_recv_buff[i], test_send_buff[i]);

}




