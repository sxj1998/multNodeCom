#include "getTimeStamp.h"

#include <stdlib.h>
#include <string.h>
#include <time.h>

char* get_current_timestamp(void) {
    time_t current_time = time(NULL);
    char* c_time_string = ctime(&current_time);
    c_time_string[strlen(c_time_string) - 1] = '\0';
    return c_time_string;
}
