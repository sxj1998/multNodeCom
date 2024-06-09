#include "bus_serial_driver.h"
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

int main(int argc, char **argv)
{
    bus_serial_driver_t* serial = bus_serial_driver_register("/dev/ttyUSB4", BUS_ID_USB0);
    bus_init((bus_driver_t**)serial);
    route_item_t* route_item = route_item_register();
    route_options_interface_i* route_opt = route_item->interface;
    route_opt->bind(route_item, (bus_driver_t**)serial);

    bus_serial_driver_t* serial1 = bus_serial_driver_register("/dev/ttyUSB5", BUS_ID_USB1);
    bus_init((bus_driver_t**)serial1);
    route_item_t* route_item1 = route_item_register();
    route_options_interface_i* route_opt1 = route_item1->interface;
    route_opt->bind(route_item1, (bus_driver_t**)serial1);

    route_ctrl_t* route_ctrl = route_ctrl_init(BOARD_ID_USB0);
    route_ctrl_add_node(route_ctrl, route_item);

    route_ctrl_t* route_ctrl1 = route_ctrl_init(BOARD_ID_USB1);
    route_ctrl_add_node(route_ctrl1, route_item1);

    route_table_add(route_ctrl, BUS_ID_USB0, BOARD_ID_USB1);
    route_table_add(route_ctrl1, BUS_ID_USB1, BOARD_ID_USB0);
    

    pthread_create(&thread_recv_sync_id,NULL,thread_recv_sync, serial);
    pthread_detach(thread_recv_sync_id);

    pthread_create(&thread_write_sync_id,NULL,thread_send_sync, serial);
    pthread_detach(thread_write_sync_id);

    pthread_create(&thread_recv_sync_id1,NULL,thread_recv_sync1, serial1);
    pthread_detach(thread_recv_sync_id1);

    pthread_create(&thread_write_sync_id1,NULL,thread_send_sync1, serial1);
    pthread_detach(thread_write_sync_id1);

    pthread_create(&thread_recv_id,NULL, thread_recv, route_ctrl);
    pthread_detach(thread_recv_id);

    pthread_create(&thread_recv_id1,NULL, thread_recv1, route_ctrl1);
    pthread_detach(thread_recv_id1);

    route_opt->send((void*)route_item,BOARD_ID_USB0 , BUS_ID_USB1, 0x05, buffer, 10);

    // sleep(1);

    // route_opt1->send((void*)route_item1, 0x01, 0x05, buffer, 10);

    while(1)
    {
        // bus_write((bus_driver_t**)serial,buffer,10);
        
        sleep(1);
    }
    
    return 0;
}
