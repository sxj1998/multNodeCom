#include "bus_socket_driver.h"
#include "utilsPrintf.h"
#include "utilsAssert.h"
#include "route_interface.h"
#include "route_handle.h"

#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h> 

uint8_t buffer[10] = {1,2,3,4,5,6,7,8,9,10};
uint8_t buffer_recv_data[256] = {1,2,3,4,5,6,7,8,9,10};

pthread_t thread_recv_sync_id, thread_write_sync_id, thread_recv_sync_id1, thread_write_sync_id1, thread_recv_id, thread_recv_id1;
pthread_t thread_wait_client_connect_thread_id, thread_client_write_test_id, thread_server_recv_test_id;

void* thread_wait_client_connect(void* arg)
{
    bus_socket_driver_t* socket = (bus_socket_driver_t*)arg;
    while (1)
    {
        int ret = bus_open((bus_driver_t**)socket);
        usleep(10*1000);
        if(ret == 0)
            return 0;
    }
}

void* thread_recv_sync(void* arg)
{
    bus_socket_driver_t* socket = (bus_socket_driver_t*)arg;
    while (1)
    {
        bus_sync_rx((bus_driver_t**)socket);
        usleep(10*1000);
    }
}

void* thread_send_sync(void* arg)
{
    bus_socket_driver_t* socket = (bus_socket_driver_t*)arg;
    while (1)
    {
        bus_sync_tx((bus_driver_t**)socket);
        usleep(10*1000);
    }
}

void* thread_recv_sync1(void* arg)
{
    bus_socket_driver_t* socket = (bus_socket_driver_t*)arg;
    while (1)
    {
        bus_sync_rx((bus_driver_t**)socket);
        usleep(10*1000);
    }
}

void* thread_send_sync1(void* arg)
{
    bus_socket_driver_t* socket = (bus_socket_driver_t*)arg;
    while (1)
    {
        bus_sync_tx((bus_driver_t**)socket);
        usleep(10*1000);
    }
}

void* thread_client_write_test(void* arg)
{
    bus_socket_driver_t* socket = (bus_socket_driver_t*)arg;
    while (1)
    {
        bus_write((bus_driver_t**)socket ,buffer, 10);
        usleep(1000*1000);
    }
}

void* thread_server_recv_test(void* arg)
{
    bus_socket_driver_t* socket = (bus_socket_driver_t*)arg;
    while (1)
    {
        int ret_read = bus_read((bus_driver_t**)socket ,buffer_recv_data, 128);
        if(ret_read > 0){
            for(int i=0; i<ret_read; i++){
                printf("%d ",buffer_recv_data[i]);
            }
            printf("\n");
        }
        usleep(10*1000);
    }
}

int main(int argc, char **argv)
{
    bus_socket_driver_t* socket_server = bus_socket_driver_register("socket_server", BUS_ID_USB1, "127.0.0.1", 8888, SOCKET_SERVER_TYPE);
    bus_init((bus_driver_t**)socket_server);
    // bus_open((bus_driver_t**)socket_server);

    pthread_create(&thread_wait_client_connect_thread_id,NULL,thread_wait_client_connect, socket_server);
    pthread_detach(thread_wait_client_connect_thread_id);

    sleep(1);

    bus_socket_driver_t* socket_client = bus_socket_driver_register("socket_client", BUS_ID_USB0, "127.0.0.1", 8888, SOCKET_CLIENT_TYPE);
    bus_init((bus_driver_t**)socket_client);
    int ret_client_con = bus_open((bus_driver_t**)socket_client);
    printf("client connected flag %d\n", ret_client_con);


    pthread_create(&thread_recv_sync_id,NULL,thread_recv_sync, socket_client);
    pthread_detach(thread_recv_sync_id);

    pthread_create(&thread_write_sync_id,NULL,thread_send_sync, socket_client);
    pthread_detach(thread_write_sync_id);

    pthread_create(&thread_recv_sync_id1,NULL,thread_recv_sync1, socket_server);
    pthread_detach(thread_recv_sync_id1);

    pthread_create(&thread_write_sync_id1,NULL,thread_send_sync1, socket_server);
    pthread_detach(thread_write_sync_id1);

    pthread_create(&thread_client_write_test_id,NULL,thread_client_write_test, socket_client);
    pthread_detach(thread_client_write_test_id);

    pthread_create(&thread_server_recv_test_id,NULL,thread_server_recv_test, socket_server);
    pthread_detach(thread_server_recv_test_id);


    bus_write((bus_driver_t**)socket_client ,buffer, 10);


    // sleep(1);

    // route_opt1->send((void*)route_item1, 0x01, 0x05, buffer, 10);

    while(1)
    {
        // bus_write((bus_driver_t**)socket_client,"buffer---------",30);
        

        sleep(1);
    }
    
    return 0;
}
