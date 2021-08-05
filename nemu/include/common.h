#ifndef __COMMON_H__
#define __COMMON_H__

// #define DEBUG
// #define DIFF_TEST
// #define TIMER_ITERRUPT_ENABLE

#if _SHARE
// do not enable these features while building a reference design
#undef DIFF_TEST
#undef DEBUG
#endif

/* You will define this macro in PA2 */
#define HAS_IOE

#include <stdint.h>
#include <assert.h>
#include <string.h>
#include <errno.h>

typedef uint8_t bool;
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef uint32_t rtlreg_t;

typedef uint32_t paddr_t;
typedef uint32_t vaddr_t;

typedef uint16_t ioaddr_t;

#define false 0
#define true 1

#include "debug.h"
#include "macro.h"

i32 parse_integer(char* arg, i64* val);

#endif
