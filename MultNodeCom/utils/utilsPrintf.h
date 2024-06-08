#ifndef __UTILS_PRINTF_H__
#define __UTILS_PRINTF_H__

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

char * timeString(void);

#define TI_DEBUG(format, ...) printf("[%s] D %s : <%s> " format "\n", timeString(), TAG, __FUNCTION__, ##__VA_ARGS__)

void utils_buff_print(uint8_t *buff, uint32_t len);

#ifdef __cplusplus
}
#endif

#endif /* __UTILS_PRINTF_H__ */
