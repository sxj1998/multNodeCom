#include "utilsAssert.h"

#include <stdint.h>
#include <stdio.h>


void _assert(const char *str_, const char *time, const char *tag)
{
    if(time != NULL)
        INFO("[%s] Assert failure!\r\n",time);
    INFO("[%s] %s %d.\r\n", tag,__FILE__,__LINE__);
    if (str_ != NULL)
        INFO("Assert info: %s.\r\n", str_);
    while (1)
    {
    }
}