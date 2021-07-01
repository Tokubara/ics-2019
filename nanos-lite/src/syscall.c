#include "common.h"
#include "syscall.h"

_Context* do_syscall(_Context *c) {
  uintptr_t a[4];
  a[0] = c->GPR1;

  switch (a[0]) {
    case SYS_exit: {
                      // printf("exit: exit_number=%d\n", c->GPR2);
                      _halt(c->GPR2);
                      break;
                    }
    case SYS_yield: {
                      _yield();
                      c->GPR1 = 0;
                      break;
                    }
    case SYS_write: {
                      if (c->GPR2 == 1 || c->GPR2 == 2) {
                        char* buf = (char*)c->GPR3;
                        size_t len = c->GPR4;
                        for(size_t i = 0; i < len; i++) {
                          _putc(buf[i]);
                        }
                        c->GPR1 = c->GPR4;
                      } else {
                        printf("unsupport\n");
                      }
                      break;
                    }
    default: panic("Unhandled syscall ID = %d", a[0]);
  }

  return NULL;
}
