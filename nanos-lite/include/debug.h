#ifndef __DEBUG_H__
#define __DEBUG_H__

#include "common.h"

#define Log(format, ...) \
  printk("\33[1;35m[%s,%d,%s] " format "\33[0m\n", \
      __FILE__, __LINE__, __func__, ## __VA_ARGS__)

#ifdef TRACE
#define LLog(format, ...) \
  printk("\33[1;35m[%s,%d,%s] " format "\33[0m\n", \
      __FILE__, __LINE__, __func__, ## __VA_ARGS__)
#else
#define LLog(format, ...) (void)0
#endif

#define panic(format, ...) \
  do { \
    Log("\33[1;31msystem panic: " format, ## __VA_ARGS__); \
    _halt(1); \
  } while (0)

#ifdef assert
# undef assert
#endif

#define assert(cond) \
  do { \
    if (!(cond)) { \
      panic("Assertion failed: %s", #cond); \
    } \
  } while (0)

#define TODO() panic("please implement me")

#define Log_debug(format, ...) \
  printf("\33[0;32m[debug][%d,%s] " format "\33[0m\n", \
      __LINE__, __func__, ## __VA_ARGS__)

#define Log_error(format, ...) \
  printf("\33[0;31m[error][%d,%s] " format "\33[0m\n", \
      __LINE__, __func__, ## __VA_ARGS__)

#define Log_info(format, ...) \
  printf("\33[0;34m[info][%d,%s] " format "\33[0m\n", \
      __LINE__, __func__, ## __VA_ARGS__)

#define LOG_TRACE
#ifdef LOG_TRACE
#define Log_trace(format, ...) \
  printf("\33[0;90m[trace][%d,%s] " format "\33[0m\n", \
      __LINE__, __func__, ## __VA_ARGS__)
#else
// #define Log_debug(format, ...)
// #define Log_info(format, ...)
// #define Log_error(format, ...)
#define Log_trace(format, ...)
#endif
#endif
