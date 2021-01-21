#include "common.h"
#include <stdarg.h>

FILE *log_fp = NULL;
/*
  如果为空, 是不会进行初始化的
*/
void init_log(const char *log_file) {
  if (log_file == NULL) return;
  log_fp = fopen(log_file, "w");
  Assert(log_fp, "Can not open '%s'", log_file);
}

char log_bytebuf[80] = {};
char log_asmbuf[80] = {};
static char tempbuf[256] = {};
/**
 * buf:已有的字符串指针buf, 一个格式字符串, 格式字符串需要传的参数, 将buf与格式化化后的字符串拼在一起
*/
void strcatf(char *buf, const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vsnprintf(tempbuf, sizeof(tempbuf), fmt, ap);
  va_end(ap);
  strcat(buf, tempbuf);
}

void asm_print(vaddr_t ori_pc, int instr_len, bool print_flag) {
  snprintf(tempbuf, sizeof(tempbuf), "%8x:   %s%*.s%s", ori_pc, log_bytebuf,
      50 - (12 + 3 * instr_len), "", log_asmbuf);
  log_write("%s\n", tempbuf);
  if (print_flag) {
    puts(tempbuf);
  }
}

void log_clearbuf(void) {
  log_bytebuf[0] = '\0';
  log_asmbuf[0] = '\0';
}
