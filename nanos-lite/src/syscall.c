#include "common.h"
#include "syscall.h"
#include "fs.h"
#include "proc.h"
#include "amdev.h"

extern PCB *current;

int sys_execve(const char *name, char *const argv[], char *const env[]);
_Context* do_syscall(_Context *c) {
  _Context* ret = NULL;
  uintptr_t a[4];
  a[0] = c->GPR1;

  switch (a[0]) {
    case SYS_exit: {
                      LLog("exit: exit_number=%d\n", c->GPR2);
                      current->status = EXITED;
                      Log_info("A job done!");
                      ret = schedule(c);
                      // naive_uload(NULL, "/bin/init");
                      // _halt(c->GPR2);
                      break;
                    }
    case SYS_yield: {
                      LLog("yield");
                      _yield();
                      c->GPRx = 0;
                      break;
                    }
    case SYS_open: {
                      LLog("open");
                      c->GPRx = fs_open(c->GPR2, c->GPR3, c->GPR4);
                      break;
                   }
    case SYS_read: {
                      LLog("read");
                      c->GPRx = fs_read(c->GPR2, c->GPR3, c->GPR4);
                      break;
                   }
    case SYS_write: {
                      LLog("write");
                      c->GPRx = fs_write(c->GPR2, c->GPR3, c->GPR4);
                      break;
                    }
    case SYS_close: {
                      LLog("close");
                      c->GPRx = fs_close(c->GPR2);
                      break;
                    }
    case SYS_lseek: {
                      LLog("lseek");
                      c->GPRx = fs_lseek(c->GPR2, c->GPR3, c->GPR4);
                      break;
                    }
    case SYS_brk: {
                    LLog("brk");
                    mm_brk(c->as, current->max_brk, c->GPR2);
                    current->max_brk = max(current->max_brk, c->GPR2);
                    c->GPRx = 0; // 表示成功
                    break;
                  }
    case SYS_execve: {
                       LLog("execve");
                       c->GPRx = sys_execve(c->GPR2, c->GPR3, c->GPR4);
                       // panic("coundn't be here");
                       ret = current->cp;
                       break;
                     }
    case SYS_gettimeofday: {
                             uint32_t cur_time = uptime(); // 以毫秒为单位
                             struct timeval* tv = (struct timeval*)c->GPR2;
                             tv->tv_sec = cur_time / 1000;
                             tv->tv_usec = ((cur_time) % 1000) * 1000;
                             c->GPRx = 0;
                             break;
                           }
    default: panic("Unhandled syscall ID = %d", a[0]);
  }

  return ret;
}
