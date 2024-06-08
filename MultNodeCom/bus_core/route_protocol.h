#ifndef F1F9FA53_6578_40C4_AD13_81C493A7EDB2
#define F1F9FA53_6578_40C4_AD13_81C493A7EDB2
#ifndef __ROUTE_PROTOCOL_H__
#define __ROUTE_PROTOCOL_H__

#include <stdint.h>

/*
**  包头    源ID    目的ID    命令    字节数    [数据]    CRC
*/
#define DATA_MAX_SIZE 256
#define MAX_PACKAGE_SIZE  DATA_MAX_SIZE+HEAD_TO_DATA_SIZE+DATA_TO_TAIL_SIZE

#define HEAD_TO_DATA_SIZE 7
#define DATA_TO_TAIL_SIZE 2

#define ROUTE_HEAD0     0x5A
#define ROUTE_HEAD1     0xA5

/*协议元素在协议中的相对位置*/
#define HEAD0_INDEX     0
#define HEAD1_INDEX     1
#define SRC_INDEX       2
#define DST_INDEX       3
#define CMD_INDEX       4
#define LEN_INDEX       5
/*协议元素在协议中[数据]后相对位置*/
#define CRC_INDEX0      0
#define CRC_INDEX1      1

int8_t pack_route_data(uint8_t* packed_buffer, uint16_t* packed_len, uint8_t* data, uint16_t len, uint8_t src_id, uint8_t dst_id, uint8_t cmd);

int8_t unpack_route_data(uint8_t* unpacked_buffer, uint16_t* unpacked_len, uint8_t* data, uint16_t len, uint8_t* src_id, uint8_t* dst_id, uint8_t* cmd);

int8_t data_unpacked(uint8_t* unpacked_buffer, uint16_t* unpacked_len, uint8_t* data, uint16_t len, uint8_t* src_id, uint8_t* dst_id, uint8_t* cmd);


#endif /* __ROUTE_PROTOCOL_H__ */


#endif /* F1F9FA53_6578_40C4_AD13_81C493A7EDB2 */
