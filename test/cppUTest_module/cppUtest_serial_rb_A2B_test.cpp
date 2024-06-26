#include "CppUTest/TestHarness.h"
#include "bus_serial_driver.h"
#include "route_interface.h"
#include "route_handle.h"
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

pthread_t thread_recv_sync_id, thread_write_sync_id, thread_recv_sync_id1, thread_write_sync_id1, thread_recv_id, thread_recv_id1;

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

void* thread_recv_sync1(void* arg)
{
    bus_serial_driver_t* serial = (bus_serial_driver_t*)arg;
    while (1)
    {
        bus_sync_rx((bus_driver_t**)serial);
        usleep(1*1000);
    }
}

void* thread_send_sync1(void* arg)
{
    bus_serial_driver_t* serial = (bus_serial_driver_t*)arg;
    while (1)
    {
        bus_sync_tx((bus_driver_t**)serial);
        usleep(1*1000);
    }
}

void* thread_recv(void* arg)
{
    uint8_t buffer[256];
    route_ctrl_t* route_ctrl = (route_ctrl_t*)arg;
    while (1)
    {
        // bus_read((bus_driver_t**)serial, buffer, 256);    
        routeRecvDataProc(route_ctrl);        
        usleep(1*1000);
    }
}

void* thread_recv1(void* arg)
{
    uint8_t buffer[256];
    route_ctrl_t* route_ctrl = (route_ctrl_t*)arg;
    while (1)
    {
        // bus_read((bus_driver_t**)serial, buffer, 256);    
        routeRecvDataProc(route_ctrl);        
        usleep(1*1000);
    }
}

uint8_t buffer[10] = {1,2,3,4,5,6,7,8,9,10};

int test1(uint8_t *buffer, uint16_t len)
{
    printf("test1\r\n");
    return 0;
}

int test2(uint8_t *buffer, uint16_t len)
{
    printf("test2\r\n");
    return 0;
}

#define  USART_DEV_A    "/dev/ttyUSB0"
#define  TEST_BUFFER_NUM_A  255
uint8_t test_send_buff_A[TEST_BUFFER_NUM_A] = {0};
uint8_t test_recv_buff_A[TEST_BUFFER_NUM_A] = {0};
bus_serial_driver_t* serial_A = NULL;
route_options_interface_i* route_opt_A = NULL;
route_item_t* route_item_A = NULL;
route_ctrl_t* route_ctrl_A = NULL;

#define  USART_DEV_B    "/dev/ttyUSB1"
#define  TEST_BUFFER_NUM_B  255
uint8_t test_send_buff_B[TEST_BUFFER_NUM_B] = {0};
uint8_t test_recv_buff_B[TEST_BUFFER_NUM_B] = {0};
bus_serial_driver_t* serial_B = NULL;
route_options_interface_i* route_opt_B = NULL;
route_item_t* route_item_B = NULL;
route_ctrl_t* route_ctrl_B = NULL;

TEST_GROUP(cppUtest_serial_A2B_test){
    void setup(){
      for(int i=0; i<TEST_BUFFER_NUM_A; i++)
      {
         test_send_buff_A[i] = i;
         test_send_buff_B[i] = i;
      }


        serial_A = bus_serial_driver_register(USART_DEV_A, BUS_ID_USB0);
        CHECK_EQUAL(serial_A->bus_driver->bus_id, BUS_ID_USB0);
        STRCMP_EQUAL(serial_A->dev_name, USART_DEV_A);
        bus_init((bus_driver_t**)serial_A);
        route_item_A = route_item_register();
        route_opt_A = route_item_A->interface;
        route_opt_A->bind(route_item_A, (bus_driver_t**)serial_A);

        serial_B = bus_serial_driver_register(USART_DEV_B, BUS_ID_USB1);
        CHECK_EQUAL(serial_B->bus_driver->bus_id, BUS_ID_USB1);
        STRCMP_EQUAL(serial_B->dev_name, USART_DEV_B);
        bus_init((bus_driver_t**)serial_B);
        route_item_B = route_item_register();
        route_opt_B = route_item_B->interface;
        route_opt_B->bind(route_item_B, (bus_driver_t**)serial_B);

        route_ctrl_A = route_ctrl_init(BOARD_ID_USB0);
        route_ctrl_add_node(route_ctrl_A, route_item_A);

        route_ctrl_B = route_ctrl_init(BOARD_ID_USB1);
        route_ctrl_add_node(route_ctrl_B, route_item_B);

        route_table_add(route_ctrl_A, BUS_ID_USB0, BOARD_ID_USB1);
        route_table_add(route_ctrl_B, BUS_ID_USB1, BOARD_ID_USB0);
        register_route_callback(route_ctrl_A, test1);
        register_route_callback(route_ctrl_B, test2);


        pthread_create(&thread_recv_sync_id,NULL,thread_recv_sync, serial_A);
        pthread_detach(thread_recv_sync_id);

        pthread_create(&thread_write_sync_id,NULL,thread_send_sync, serial_A);
        pthread_detach(thread_write_sync_id);

        pthread_create(&thread_recv_sync_id1,NULL,thread_recv_sync1, serial_B);
        pthread_detach(thread_recv_sync_id1);

        pthread_create(&thread_write_sync_id1,NULL,thread_send_sync1, serial_B);
        pthread_detach(thread_write_sync_id1);

        pthread_create(&thread_recv_id,NULL, thread_recv, route_ctrl_A);
        pthread_detach(thread_recv_id);

        pthread_create(&thread_recv_id1,NULL, thread_recv1, route_ctrl_B);
        pthread_detach(thread_recv_id1);


    }

    void teardown(){
         int ret = bus_close((bus_driver_t**)serial_A);
         CHECK_EQUAL(ret, 0);

         ret = bus_close((bus_driver_t**)serial_B);
         CHECK_EQUAL(ret, 0);
    }
};


TEST(cppUtest_serial_A2B_test, check_send_recv)
{
    while(1){
        route_opt_A->send((void*)route_item_A, BOARD_ID_USB0 , BOARD_ID_USB1, 0x05, buffer, 10);
        sleep(1);
    }

}




