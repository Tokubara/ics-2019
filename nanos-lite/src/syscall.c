#include "common.h"
#include "syscall.h"
#include "fs.h"

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
                      c->GPRx = 0;
                      break;
                    }
    case SYS_open: {
                      c->GPRx = fs_open(c->GPR2, c->GPR3, c->GPR4);
                      break;
                   }
    case SYS_read: {
                      c->GPRx = fs_read(c->GPR2, c->GPR3, c->GPR4);
                      break;
                   }
    case SYS_write: {
                      c->GPRx = fs_write(c->GPR2, c->GPR3, c->GPR4);
                      break;
                    }
    case SYS_close: {
                      c->GPRx = fs_close(c->GPR2);
                      break;
                    }
    case SYS_lseek: {
                      c->GPRx = fs_lseek(c->GPR2, c->GPR3, c->GPR4);
                      break;
                    }
    case SYS_brk: {
                    c->GPRx = 0; // 表示成功
                    break;
                  }
    default: panic("Unhandled syscall ID = %d", a[0]);
  }

  return NULL;
}
