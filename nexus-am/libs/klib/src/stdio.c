#include "klib.h"
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)


char int_buf[20]; // 最大的long long十进制长度为19
// 返回长度
int fill_int_buf(unsigned long long lval) {
  int i = 0;
  if(lval==0) {
    int_buf[0]='0';
    int_buf[1]='\0';
    return 1;
  }
  char tmp;
  while (lval > 0) {
    int_buf[i++] = (lval % 10) + '0';
    lval /= 10;
  }
  int left = 0;
  int right = i - 1; // 现在i是右开, 因此-1
  while (left < right) {
    tmp = int_buf[right];
    int_buf[right] = int_buf[left];
    int_buf[left] = tmp;
    ++left;
    --right;
  }
  int_buf[i]='\0';
  return i;
}

int allprintf(char *out, const char *fmt, va_list ap) {
#define out_putc(out_putc_ch)  if(out!=NULL) {\
          out[i++] = out_putc_ch;\
      } else {\
        _putc(out_putc_ch);\
      }
  size_t i = 0; // i指向out, j指向fmt
  const char* p;
  int ival;
  unsigned long long lval = 0; // 为了过编
  size_t int_buf_len;
  char tmp_ch;
  
  for (p = fmt; *p; ++p) {
    if (*p != '%') {
      out_putc(*p);
      continue;
    }

    // 处理其它情况
    ++p;
    switch (*p) {
    case 's': {
      for (char* s = va_arg(ap, char*); *s; ++s) {
        out_putc(*s);
      }
      break;
    }
    case 'd': {
      ival = va_arg(ap, int);
      if (ival < 0) {
        out_putc('-');
        lval = (-ival);
      }
      int_buf_len = fill_int_buf(lval);
      for(size_t i = 0; i<int_buf_len; i++) {
        tmp_ch = int_buf[i];
        out_putc(tmp_ch);
      }
      i+=int_buf_len;
      break;
    }
    default: {
      return -1;
    }
    }
  }

  out[i] = '\0';
  return i;
#undef out_putc
}


int printf(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  allprintf(NULL, fmt, ap);
  va_end(ap);
  return 0;
}

int vsprintf(char *out, const char *fmt, va_list ap) {
  return 0;
}

int sprintf(char* out, const char* fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  int ret = allprintf(out, fmt, ap);
  va_end(ap);
  return ret;
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  return 0;
}

#endif
