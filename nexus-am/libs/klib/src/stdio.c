#include "klib.h"
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

int printf(const char *fmt, ...) {
  return 0;
}

int vsprintf(char *out, const char *fmt, va_list ap) {
  return 0;
}

int sprintf(char* out, const char* fmt, ...) {
  va_list ap;
  va_start(ap, fmt);

  size_t i = 0; // i指向out, j指向fmt
  const char* p;
  int ival;
  long long lval;
  char tmp;
  for (p = fmt; *p; ++p) {
    if (*p != '%') {
      out[i] = *p;
      ++i;
      continue;
    }

    // 处理其它情况
    ++p;
    switch (*p) {
    case 's': {
      for (char* s = va_arg(ap, char*); *s; ++s) {
        out[i] = *s;
        ++i;
      }
      break;
    }
    case 'd': {
      ival = va_arg(ap, int);
      lval = ival;
      if (lval < 0) {
        out[i] = '-';
        ++i;
        lval = -lval;
      }
      int left = i;
      while (lval > 0) {
        out[i++] = (lval % 10) + '0';
        lval /= 10;
      }
      int right = i - 1; // 现在i是右开
      while (left < right) {
        tmp = out[right];
        out[right] = out[left];
        out[left] = tmp;
        ++left;
        --right;
      }
      break;
    }
    default: {
      return -1;
    }
    }
  }

  out[i] = '\0';
  return i;
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  return 0;
}

#endif
