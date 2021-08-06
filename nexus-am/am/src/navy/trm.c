#include <am.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h> // 发现了exit
#include <assert.h> // 提供了assert


extern char _heap_start;
extern char _heap_end;
_Area _heap = {
  .start = &_heap_start,
  .end = &_heap_end,
};
int main(int argc, char *argv[], char *envp[]);
void _trm_init() {
  assert(0);
}

void _putc(char ch) {
  putchar(ch);
}

void _halt(int code) {
  exit(code);
}
