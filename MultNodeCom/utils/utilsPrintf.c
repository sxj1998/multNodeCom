#include "utilsAssert.h"
#include "utilsPrintf.h"
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>

char * timeString(void) {
  struct timespec ts;
  clock_gettime( CLOCK_REALTIME, &ts);
  struct tm * timeinfo = localtime(&ts.tv_sec);
  static char timeStr[20];
  sprintf(timeStr, "%.2d-%.2d %.2d:%.2d:%.2d.%.3ld", timeinfo->tm_mon + 1, timeinfo->tm_mday, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec, ts.tv_nsec / 1000000);
  return timeStr;
}
 
void utils_buff_print(uint8_t *buff, uint32_t len)
{
  for(int i=0; i<len; i++)
  {
    printf("%02x ", buff[i]);
  }
  printf("\r\n");
}