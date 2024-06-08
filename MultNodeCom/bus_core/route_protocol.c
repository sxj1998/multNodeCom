#include "route_protocol.h"
#include "crc16.h"

#include <string.h>

int8_t pack_route_data(uint8_t* packed_buffer, uint16_t* packed_len, uint8_t* data, uint16_t len, uint8_t src_id, uint8_t dst_id, uint8_t cmd)
{
    uint16_t crc16_res = 0;
    packed_buffer[HEAD0_INDEX] = ROUTE_HEAD0;
    packed_buffer[HEAD1_INDEX] = ROUTE_HEAD1;
    packed_buffer[SRC_INDEX] = src_id;
    packed_buffer[DST_INDEX] = dst_id;
    packed_buffer[CMD_INDEX] = cmd;
    packed_buffer[LEN_INDEX] = (len>>8)&0xff;
    packed_buffer[LEN_INDEX+1] = len&0xff;
    memcpy(packed_buffer + HEAD_TO_DATA_SIZE, data, len);
    crc16_res = crc16((const char*)packed_buffer ,HEAD_TO_DATA_SIZE + len);
    packed_buffer[HEAD_TO_DATA_SIZE + len + CRC_INDEX0] = (crc16_res >> 8) & 0xff; 
    packed_buffer[HEAD_TO_DATA_SIZE + len + CRC_INDEX1] = crc16_res  & 0xff; 
    *packed_len = HEAD_TO_DATA_SIZE + len + DATA_TO_TAIL_SIZE;
    return 0;
}


int8_t unpack_route_data(uint8_t* unpacked_buffer, uint16_t* unpacked_len, uint8_t* data, uint16_t len, uint8_t* src_id, uint8_t* dst_id, uint8_t* cmd)
{
    for(int i =0 ; i < len ; i++)
    {
       if(data_unpacked(unpacked_buffer, unpacked_len, data + i, len - i, src_id, dst_id, cmd) == 0)
            return 0;
    }
    return -1;
}

int8_t data_unpacked(uint8_t* unpacked_buffer, uint16_t* unpacked_len, uint8_t* data, uint16_t len, uint8_t* src_id, uint8_t* dst_id, uint8_t* cmd)
{
    uint16_t length = 0, crc16_res = 0;

    if((data[HEAD0_INDEX] != ROUTE_HEAD0) || (data[HEAD1_INDEX] != ROUTE_HEAD1))
        return -1;

    *src_id = data[SRC_INDEX]; 
    *dst_id = data[DST_INDEX];
    *cmd = data[CMD_INDEX];
    *unpacked_len = (data[LEN_INDEX]<< 8) + data[LEN_INDEX + 1];
    crc16_res = (data[HEAD_TO_DATA_SIZE + *unpacked_len + CRC_INDEX0]<< 8) + data[HEAD_TO_DATA_SIZE + *unpacked_len + CRC_INDEX1];

    if(crc16(data,HEAD_TO_DATA_SIZE + *unpacked_len) != crc16_res)
        return -2;

    memcpy(unpacked_buffer, data + HEAD_TO_DATA_SIZE, *unpacked_len);

    return 0;
}