#include "bus_serial_driver.h"
#include "utilsPrintf.h"
#include "utilsAssert.h"
#include "route_interface.h"

#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h> 

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

void* thread_recv(void* arg)
{
    uint8_t buffer[256];
    bus_serial_driver_t* serial = (bus_serial_driver_t*)arg;
    while (1)
    {
        bus_read((bus_driver_t**)serial, buffer, 256);            
        usleep(1*1000);
    }
}

uint8_t buffer[10] = {1,2,3,4,5,6,7,8,9,10};

int main(int argc, char **argv)
{
    bus_serial_driver_t* serial = bus_serial_driver_register("/dev/ttyUSB3",1);
    bus_init((bus_driver_t**)serial);

    route_item_t* route_item = route_item_register();
    route_options_interface_i* route_opt = route_item->interface;
    route_opt->bind(route_item, (bus_driver_t**)serial);

    pthread_create(&thread_recv_sync_id,NULL,thread_recv_sync, serial);
    pthread_detach(thread_recv_sync_id);

    pthread_create(&thread_write_sync_id,NULL,thread_send_sync, serial);
    pthread_detach(thread_write_sync_id);

    pthread_create(&thread_recv_id,NULL, thread_recv, serial);
    pthread_detach(thread_recv_id);

    while(1)
    {
        // bus_write((bus_driver_t**)serial,buffer,10);
        route_opt->send((void*)route_item, 0x03, 0x05, buffer, 10);
        sleep(1);
    }
    
    return 0;
}
