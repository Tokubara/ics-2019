#ifndef __DEBUG_H__
#define __DEBUG_H__

#define panic(format, ...) \
  do { \
    Log("\33[1;31msystem panic: " format, ## __VA_ARGS__); \
    _halt(1); \
  } while (0)

#define Log(format, ...) \
  printf("\33[1;35m[%s,%d,%s] " format "\33[0m\n", \
      __FILE__, __LINE__, __func__, ## __VA_ARGS__)

#define Log_trace(format, ...) \
  printf("\33[0;90m[debug][%d,%s] " format "\33[0m\n", \
      __LINE__, __func__, ## __VA_ARGS__)

#define Log_debug(format, ...) \
  printf("\33[0;32m[debug][%d,%s] " format "\33[0m\n", \
      __LINE__, __func__, ## __VA_ARGS__)

#define Log_error(format, ...) \
  printf("\33[0;31m[error][%d,%s] " format "\33[0m\n", \
      __LINE__, __func__, ## __VA_ARGS__)

#define assert(cond) \
  do { \
    if (!(cond)) { \
      panic("Assertion failed: %s", #cond); \
    } \
  } while (0)

#define TODO() panic("please implement me")

typedef uint8_t bool;
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

#endif
