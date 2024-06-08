#ifndef __UTILS_ASSERT_H__
#define __UTILS_ASSERT_H__

#include "getTimeStamp.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MODULE_TAG(_tag)                  static const char *TAG = _tag

#define INFO    printf

#define utils_assert(test_)                  do                                 \
    {                                                                          \
        if (!(test_))                                                          \
            _assert(#test_, get_current_timestamp(), TAG);                                 \
    } while (0)
    
// #define assert(test_)                       utils_assert(test_)

void _assert(const char *str_, const char *time, const char *tag);

#ifdef __cplusplus
}
#endif

#endif 
