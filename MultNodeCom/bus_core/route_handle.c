#include "bus_serial_driver.h"
#include "utilsPrintf.h"
#include "utilsAssert.h"
#include "route_interface.h"
#include "route_handle.h"

#include <stdlib.h>
#include <string.h>

MODULE_TAG("ROUTE_HANDLE"); 

static int8_t is_route_table_has_dst(uint8_t dst, uint8_t (*table)[2]);

route_ctrl_t* route_ctrl_init(uint8_t board_id)
{
    route_ctrl_t* route_ctrl = malloc(sizeof(route_ctrl_t));
    memset(route_ctrl, 0, sizeof(route_ctrl_t));    
    route_ctrl->route_ctrl_id = board_id;
    TI_DEBUG(" route ctrl %d regist success", route_ctrl->route_ctrl_id);
    return route_ctrl;
}

int route_ctrl_add_node(route_ctrl_t* route_ctrl, route_item_t* route_item)
{
    bus_driver_t* bus_driver = *(route_item->bus);

    uint8_t bus_index = route_ctrl->bus_index;
    route_ctrl->route_item[bus_index] = route_item;
    route_ctrl->route_bus_id[bus_index] = bus_driver->bus_id;
    TI_DEBUG(" route bus id add index: %d", route_ctrl->bus_index);
    route_ctrl->bus_index++;
    utils_assert(BUS_NUM_MAX >= route_ctrl->bus_index);
    return bus_index;
}

void routeRecvDataProc(route_ctrl_t* route_ctrl)
{
    for(int i = 0; i < route_ctrl->bus_index; i++){
        route_item_t* route = route_ctrl->route_item[i];
        utils_assert(route);

        bus_driver_t* bus_driver = *(route->bus);
        utils_assert(bus_driver);

        route_options_interface_i* route_opt = route->interface;
        uint16_t len = route_opt->recv(route, (uint8_t*)&route->buffer[P_READ], MAX_PACKAGE_SIZE);
        if(len == 0)
            return ;

        int8_t ret = unpack_route_data(route->package[P_READ].buffer, &route->package[P_READ].length, (uint8_t*)&route->buffer[P_READ], len, \
                                        &route->package[P_READ].src_id, &route->package[P_READ].dst_id, &route->package[P_READ].cmd);
        if(ret == 0){
            TI_DEBUG("ROUTE RX %d: src_id 0x%02x dst_id 0x%02x cmd 0x%02x",route->package[P_READ].length, route->package[P_READ].src_id ,route->package[P_READ].dst_id , route->package[P_READ].cmd);
            TI_DEBUG("ROUTE BUS : %s id : %d",bus_driver->bus_name, bus_driver->bus_id);
            int8_t dst_index = is_route_table_has_dst(route->package[P_READ].dst_id, route_ctrl->route_table);

            if(dst_index >= 0){
                route_item_t* route_dst = route_ctrl->route_item[dst_index];
                utils_assert(route_dst);
                route_options_interface_i* route_dst_opt = route_dst->interface;
                route_dst_opt->send(route_dst, route_ctrl->route_ctrl_id, route->package[P_READ].dst_id, route->package[P_READ].cmd ,route->package[P_READ].buffer, route->package[P_READ].length);
            }
            // route_opt->send(route, route_ctrl->route_ctrl_id, route_ctrl->route_table[dst_index], route->package[P_READ].cmd ,route->package[P_READ].buffer, route->package[P_READ].length);
            // utils_buff_print(route->package[P_READ].buffer, route->package[P_READ].length);
        }
    }
}

static int8_t is_route_table_has_dst(uint8_t dst, uint8_t (*table)[2])
{
    for(int i = 0; i<BUS_NUM_MAX; i++){
        if(dst == table[i][ROUTE_BOARD])
            return i;
    }
    return -1;
}

int route_table_add(route_ctrl_t* route_ctrl, BUS_ID_TYPE bus_id, BOARD_ID_TYPE board_id)
{
    route_ctrl->route_table[route_ctrl->route_index][ROUTE_BUS] = bus_id;
    route_ctrl->route_table[route_ctrl->route_index][ROUTE_BOARD] = board_id;
    TI_DEBUG(" route table add index: %d , bus_id %02x, board_id %02x", route_ctrl->route_index, route_ctrl->route_table[route_ctrl->route_index][ROUTE_BUS], route_ctrl->route_table[route_ctrl->route_index][ROUTE_BOARD]);
    route_ctrl->route_index++;
}


