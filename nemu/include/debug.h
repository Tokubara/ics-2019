#ifndef __DEBUG_H__
#define __DEBUG_H__

#include "common.h"
#include <stdio.h>
#include <assert.h>
#include "monitor/log.h"

#define Log(format, ...) \
    _Log("\33[1;34m[%s,%d,%s] " format "\33[0m\n", \
        __FILE__, __LINE__, __func__, ## __VA_ARGS__)

#define Assert(cond, ...) \
  do { \
    if (!(cond)) { \
      fflush(stdout); \
      fprintf(stderr, "\33[1;31mcpu.pc=%x, ", cpu.pc); \
      fprintf(stderr, __VA_ARGS__); \
      fprintf(stderr, "\33[0m\n"); \
      assert(cond); \
    } \
  } while (0)

#define Assert_func(cond, ...) \
  do { \
    if (!(cond)) { \
      Log(__VA_ARGS__); \
      return -1; \
    } \
  } while (0)

// 我感觉这个有点多余, 如果有更好的办法可以与Assert_func合并, 我就合并
#define assert_func(cond) \
  do { \
    if (!(cond)) { \
      return -1; \
    } \
  } while (0)

#define panic(...) Assert(0, __VA_ARGS__)

#define TODO() panic("please implement me")

// #define LOG
#ifdef LOG
#define Log_debug(format, ...) \
  printf("\33[0;32m[debug][%d,%s] " format "\33[0m\n", \
      __LINE__, __func__, ## __VA_ARGS__)

#define Log_error(format, ...) \
  printf("\33[0;31m[error][%d,%s] " format "\33[0m\n", \
      __LINE__, __func__, ## __VA_ARGS__)
#else
#define Log_debug(format, ...)
#define Log_error(format, ...)
#endif
#endif
