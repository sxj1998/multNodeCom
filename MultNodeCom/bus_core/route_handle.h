#ifndef __ROUTE_HANDLE_H__
#define __ROUTE_HANDLE_H__

#include "route_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

#define BUS_NUM_MAX     5    
#define ROUTE_NUM_MAX   5

typedef int (*route_handle_callback)(uint8_t*, uint16_t);

typedef enum{
    ROUTE_BUS = 0,
    ROUTE_BOARD = 1
}BUS_BOARD_MAP_TYPE;

typedef enum{
    BUS_ID_USB0 = 0x01,
    BUS_ID_USB1,
    BUS_ID_USB2,
    BUS_ID_USB3,
    BUS_ID_MAX
}BUS_ID_TYPE;

typedef enum{
    BOARD_ID_USB0 = 0xA0,
    BOARD_ID_USB1,
    BOARD_ID_USB2,
    BOARD_ID_USB3,
    BOARD_ID_MAX
}BOARD_ID_TYPE;

typedef struct {
    route_item_t* route_item[BUS_NUM_MAX];
    uint8_t route_bus_id[BUS_NUM_MAX];
    uint8_t bus_index;
    uint8_t route_table[ROUTE_NUM_MAX][2];
    uint8_t route_index;
    uint8_t route_ctrl_id;
    route_handle_callback route_data_handle;
}route_ctrl_t;

void routeRecvDataProc(route_ctrl_t* self);
route_ctrl_t* route_ctrl_init(uint8_t board_id);
int route_ctrl_add_node(route_ctrl_t* route_ctrl, route_item_t* route_item);
int route_table_add(route_ctrl_t* route_ctrl, BUS_ID_TYPE bus_id, BOARD_ID_TYPE board_id);
void register_route_callback(route_ctrl_t* route_ctrl, route_handle_callback cb);

#ifdef __cplusplus
}
#endif

#endif /* __ROUTE_HANDLE_H__ */
