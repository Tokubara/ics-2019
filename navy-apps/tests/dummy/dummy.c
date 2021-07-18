#include <stdint.h>
#include <assert.h>

#define SYS_yield 1
extern int _syscall_(int, uintptr_t, uintptr_t, uintptr_t);

int main(int argc, char *argv[], char *envp[]) {
  return _syscall_(SYS_yield, 0, 0, 0);
}
