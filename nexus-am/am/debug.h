#ifndef __DEBUG_H__
#define __DEBUG_H__


#define Log(format, ...) \
  printf("\33[1;35m[%s,%d,%s] " format "\33[0m\n", \
      __FILE__, __LINE__, __func__, ## __VA_ARGS__)


#define Log_debug(format, ...) \
  printf("\33[0;32m[debug][%d,%s] " format "\33[0m\n", \
      __LINE__, __func__, ## __VA_ARGS__)

#define Log_error(format, ...) \
  printf("\33[0;31m[error][%d,%s] " format "\33[0m\n", \
      __LINE__, __func__, ## __VA_ARGS__)
#endif
