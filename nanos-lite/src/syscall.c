#include "common.h"
#include "syscall.h"
#include "fs.h"

_Context* do_syscall(_Context *c) {
  uintptr_t a[4];
  a[0] = c->GPR1;

  switch (a[0]) {
    case SYS_exit: {
                      LLog("exit: exit_number=%d\n", c->GPR2);
                      _halt(c->GPR2);
                      break;
                    }
    case SYS_yield: {
                      LLog("yield\n");
                      _yield();
                      c->GPRx = 0;
                      break;
                    }
    case SYS_open: {
                      LLog("open\n");
                      c->GPRx = fs_open(c->GPR2, c->GPR3, c->GPR4);
                      break;
                   }
    case SYS_read: {
                      LLog("read\n");
                      c->GPRx = fs_read(c->GPR2, c->GPR3, c->GPR4);
                      break;
                   }
    case SYS_write: {
                      LLog("write\n");
                      c->GPRx = fs_write(c->GPR2, c->GPR3, c->GPR4);
                      break;
                    }
    case SYS_close: {
                      LLog("close\n");
                      c->GPRx = fs_close(c->GPR2);
                      break;
                    }
    case SYS_lseek: {
                      LLog("lseek\n");
                      c->GPRx = fs_lseek(c->GPR2, c->GPR3, c->GPR4);
                      break;
                    }
    case SYS_brk: {
                    LLog("brk\n");
                    c->GPRx = 0; // 表示成功
                    break;
                  }
    default: panic("Unhandled syscall ID = %d", a[0]);
  }

  return NULL;
}
