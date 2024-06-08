#include "bus_serial_driver.h"

#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h> 

pthread_t thread_recv_sync_id, thread_write_sync_id;

void* thread_recv_sync(void* arg)
{
    // route_item_t* route_item = (route_item_t*)arg;
    // serial_device_t* serial = route_item->device;
    // while (1)
    // {
    //     rb_serial_read_sync(serial);
    //     usleep(1*1000);
    // }
}

void* thread_send_sync(void* arg)
{
    // route_item_t* route_item = (route_item_t*)arg;
    // serial_device_t* serial = route_item->device;
    // while (1)
    // {
    //     rb_serial_write_sync(serial);
    //     usleep(1*1000);
    // }
}

int main(int argc, char **argv)
{
    bus_serial_driver_t* serial = bus_serial_driver_register("/dev/ttyUSB3");
    bus_init((bus_driver_t**)serial);

    // pthread_create(&thread_recv_sync_id,NULL,thread_recv_sync,serial);
    // pthread_detach(thread_recv_sync_id);

    // pthread_create(&thread_write_sync_id,NULL,thread_send_sync,serial);
    // pthread_detach(thread_write_sync_id);
    
    return 0;
}
