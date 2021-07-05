#include "common.h"
#include "syscall.h"
#include "fs.h"

_Context* do_syscall(_Context *c) {
  uintptr_t a[4];
  a[0] = c->GPR1;

  switch (a[0]) {
    case SYS_exit: {
                      // printf("exit: exit_number=%d\n", c->GPR2);
                      printf("exit\n");
                      _halt(c->GPR2);
                      break;
                    }
    case SYS_yield: {
                      printf("yield\n");
                      _yield();
                      c->GPRx = 0;
                      break;
                    }
    case SYS_open: {
                      printf("open\n");
                      c->GPRx = fs_open(c->GPR2, c->GPR3, c->GPR4);
                      break;
                   }
    case SYS_read: {
                      printf("read\n");
                      c->GPRx = fs_read(c->GPR2, c->GPR3, c->GPR4);
                      break;
                   }
    case SYS_write: {
                      printf("write\n");
                      c->GPRx = fs_write(c->GPR2, c->GPR3, c->GPR4);
                      break;
                    }
    case SYS_close: {
                      printf("close\n");
                      c->GPRx = fs_close(c->GPR2);
                      break;
                    }
    case SYS_lseek: {
                      printf("lseek\n");
                      c->GPRx = fs_lseek(c->GPR2, c->GPR3, c->GPR4);
                      break;
                    }
    case SYS_brk: {
                    printf("brk\n");
                    c->GPRx = 0; // 表示成功
                    break;
                  }
    default: panic("Unhandled syscall ID = %d", a[0]);
  }

  return NULL;
}
