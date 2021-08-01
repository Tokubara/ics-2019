#ifndef __COMMON_H__
#define __COMMON_H__

/* Uncomment these macros to enable corresponding functionality. */
#define HAS_CTE
// #define HAS_VME
// #define TRACE

#include <am.h>
#include <klib.h>
#include "debug.h"
#define max(a,b) ((a>=b)?a:b)
typedef unsigned char bool;
#define true 1
#define false 0

#endif
