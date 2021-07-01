#include "common.h"
#include "syscall.h"

_Context* do_syscall(_Context *c) {
  uintptr_t a[4];
  a[0] = c->GPR1;

  switch (a[0]) {
    case SYS_yield: {
                      printf("syscall: yield\n");
                      _yield();
                      c->GPR1 = 0;
                      break;
                    }
    case SYS_exit: {
                      printf("exit: exit_number=%d\n", c->GPR2);
                      _halt(c->GPR2);
                      break;
                    }
    default: panic("Unhandled syscall ID = %d", a[0]);
  }

  return NULL;
}
