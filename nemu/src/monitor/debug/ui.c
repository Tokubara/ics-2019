#include "monitor/expr.h"
#include "monitor/monitor.h"
#include "monitor/watchpoint.h"
#include "nemu.h"
#include "device/map.h"

#include <errno.h>
#include <readline/history.h>
#include <readline/readline.h>
#include <stdlib.h>

void cpu_exec(uint64_t);

/* We use the `readline' library to provide more flexibility to read from stdin.
 * 大概相当于readline */
static char *rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}

static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}

static int cmd_q(char *args) { return -1; }

static int cmd_help(char *args);

static int cmd_si(char *args);

static int cmd_x(char *args);

static int cmd_info(char *args);

static int cmd_info_r() {
  // 应该是打印所有寄存器吧
  for (int i = 0; i < 8; i++) {
    printf("%s  0x%.8x %u\n", reg_name(i, 4), cpu.gpr[i]._32,
           cpu.gpr[i]._32);
  }
  return 0;
}

static struct {
  char *name;
  char *description;
  int (*handler)(char *);
} cmd_table[] = {
    {"help", "Display informations about all supported commands", cmd_help},
    {"c", "Continue the execution of the program", cmd_c},
    {"q", "Exit NEMU", cmd_q},
    {"si", "Execute [N] instuctions", cmd_si},
    {"x", "Print [N] bytes of memory from [addr]", cmd_x},
    {"info", "Print useful info", cmd_info}

    /* TODO: Add more commands */

};

// enum { // 似乎框架中的enum都是小写的
//   CMD_HELP_IDX,
//   CMD_C_IDX,
//   CMD_Q_IDX,
//   CMD_SI_IDX,
//   CMD_X_IDX,
//   CMD_INFO_IDX
// }

#define NR_CMD (sizeof(cmd_table) / sizeof(cmd_table[0]))

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  } else {
    for (i = 0; i < NR_CMD; i++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

static int cmd_si(char *args) {
  // 用法就是si N, 只支持10进制
  // char* endptr;
  // bug:这个args根本不能用, 明明应该是NULL, 传了以后根本不是NULL,
  // 是strtok_r+53, H0018
  long N;
  char *arg = strtok(NULL, " ");
  if (!arg) {
    N = 1;
  } else {
    errno = 0;
    N = strtol(arg, NULL, 10);
    if (N <= 0 || errno != 0) {
      printf("N must greater than 0, and not too big\n");
      return 0;
    }
  }
  // 并不关心后面怎样
  // 需要执行n.
  cpu_exec(N);
  return 0;
}

int (*cmd_info_table[])()={cmd_info_r}; // bug:这里发生了重名错误
enum {INFO_R_IDX}; // 但是目前没用上

static int cmd_info(char *args) { 
  char *arg = strtok(NULL, " ");
  if(arg == NULL) {
    printf("%s - %s\n", cmd_table[CMD_INFO_IDX].name,
           cmd_table[CMD_INFO_IDX].description);
  } else if(!strcmp(arg, "r")) {
    cmd_info_r();
  }
  return 0;
}

static int cmd_x(char *args) { 
  char *arg = strtok(NULL, " ");
  if (arg == NULL) {
    printf("%s - %s\n", cmd_table[CMD_X_IDX].name,
          cmd_table[CMD_X_IDX].description);
    return 0;
  } 
    // 这里只支持读16进制的
  errno = 0;
  int N = strtol(arg, NULL, 10);
  if (N <= 0 || errno!=0) {
    printf("invalid N\n");
    return 0;
  }
  arg = strtok(NULL, " ");
  vaddr_t addr = strtol(arg, NULL, 16);
  if (addr <= 0 || errno != 0) {
    printf("Cannot access memory at address at 0x%x\n", addr);
    return 0;
  }
  uint32_t tmp;
  int i = 0;
  for (; i < N; i++) {
    // 没检查内存是否有效
    tmp = vaddr_read(addr, 4);
    if(i%4==0) {
      // 仿照gdb的输出, 4个word一行
      printf("0x%.8x: 0x%.8x", addr, tmp);
    } else {
      printf(" 0x%.8x%c", tmp, i % 4 == 3 ? '\n' :'\0');
    }
    addr+=4;
  }
  if(i%4!=0) putchar('\n');
  return 0;
}

void ui_mainloop(int is_batch_mode) {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  for (char *str; (str = rl_gets()) != NULL;) {
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) {
      continue;
    }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args =
        cmd + strlen(cmd) +
        1; // 首先这个+1是指通配符. 这时候不能再是str+了, 因为str已经改变了
    if (args >= str_end) {
      args = NULL;
    }

#ifdef HAS_IOE
    extern void sdl_clear_event_queue(void);
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) {
          return;
        }
        break;
      }
    }
    if (i == NR_CMD) {
      printf("Unknown command '%s'\n", cmd);
    }
  }
}
