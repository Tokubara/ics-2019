#include <stdint.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#define SYS_yield 1
extern int _syscall_(int, uintptr_t, uintptr_t, uintptr_t);

int main(int argc, char *argv[], char *envp[]) {
  int* a = malloc(100*4);
  int* b = malloc(10000*4);
  for(int i = 0; i < 100; i++) {
    a[i] = i;
  }
  for(int i = 0; i < 10000; i+=100) {
    b[i] = i;
    // assert(a[i] == i);
  }
  free(a);
  free(b);
  return 0;
  // return _syscall_(SYS_yield, 0, 0, 0);
}
