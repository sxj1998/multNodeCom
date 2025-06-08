#ifndef __CRC16_H__
#define __CRC16_H__

#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus  */

uint16_t crc16(const char *buf, int len);

uint16_t crc16_update(uint16_t crc, uint8_t data);

#ifdef __cplusplus
}
#endif /* __cplusplus  */

#endif

