#include "common.h"
#include "monitor/expr.h"
#include "monitor/monitor.h"
#include "monitor/watchpoint.h"
#include "nemu.h"
// #include "device/map.h"

// #include <_types/_uint32_t.h>
#include <readline/history.h>
#include <readline/readline.h>
#include <stdlib.h>

void cpu_exec(uint64_t);

/**
 * 输出提示(nemu), 读取一行, 不返回\n, 但是没有释放掉内存(本该释放内存的)
 */
static char *rl_gets() {
  static char *line_read = NULL; //? 这里为什么要存入static. 一般static变量是为了后续调用还能继续用 ,但是这里都置空了, 显然就没这个功能了. 会不会这是add_history的需求? 我的猜测是, add_history才不会拷贝参数字符串呢, 它只是保存个地址. 但也说不过去, 毕竟readline返回的应该就是堆内存, 本来也不会随函数的结束被销毁 

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) "); //? line_read得到的字符串是不是不包含\n?

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

static int cmd_p(char *args);
static int cmd_d(char *args);
static int cmd_w(char *args);

static struct {
  char *name;
  char *description;
  int (*handler)(char *); // 统一都是接受一整个字符串
} cmd_table[] = {
    {"help", "Display informations about all supported commands", cmd_help},
    {"c", "Continue the execution of the program", cmd_c},
    {"q", "Exit NEMU", cmd_q},
    {"si", "Execute [N] instuctions", cmd_si},
    {"x", "x N(base 10 integer) addr(expr)\nPrint [N] bytes of memory from [addr]", cmd_x},
    {"p", "p expr\nPrint an expression", cmd_p},
    {"info", "info r|w\nPrint gpr|watchpoints", cmd_info},
    {"w", "Set a watchpoint", cmd_w},
    {"d", "Delete a watchpoint", cmd_d}
};

enum { // 似乎框架中的enum都是小写的
  CMD_HELP_IDX,
  CMD_C_IDX,
  CMD_Q_IDX,
  CMD_SI_IDX,
  CMD_X_IDX,
  CMD_INFO_IDX
};

#define NR_CMD (sizeof(cmd_table) / sizeof(cmd_table[0]))

static int cmd_p(char *args) {
  i32 ret;
  if(args==NULL) {
    puts("No given expressions");
    return 0;
  }
  // expr
  uint32_t expr_val;
  ret = expr(args, &expr_val);
  if(ret<0) {
    puts("The expression is invalid");
  } else {
    printf("%s = %u\n", args, expr_val);
  }
  return 0;
}

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

/** 
 * 支持args为NULL, 相当于为"1", 只能是数, 不能是表达式(因为PA1.3有说是N)
*/
static int cmd_si(char *args) {
  // 用法就是si N, 只支持10进制
  // bug:这个args根本不能用, 明明应该是NULL, 传了以后根本不是NULL,
  // 是strtok_r+53, H0018
  if(args==NULL) {
    cpu_exec(1);
  } else {
    i32 ret;
    long N;
    ret = parse_integer(args,&N);
    if(ret==-1 || N<0) {
       puts("invalid N");
    } else {
      cpu_exec(N);
    }
  }
  return 0;
}

static int cmd_w(char *args) {
  new_wp(args);
  return 0;
}

/**
 * 解析字符串arg为整数, 失败返回-1, 否则返回0
 * 不会处理arg==NULL的情况
 * 会打印错误信息, arg==NULL不会报错(比如cmd_si有这种需求),errno不为0会报错
 * 
*/
i32 parse_integer(char* arg, i64* val) {
  Assert(arg!=NULL, "arg is NULL");
  errno = 0;
  *val = strtol(arg, NULL, 10);
  if (errno != 0) {
    Log("invalid number input");
    return -1;
  }
  return 0;
}

static int cmd_d(char *args) {
  i64 no;
  i32 ret;
  ret = parse_integer(args, &no);
  if(ret==0) {
    del_wp_NO(no);
  }
  return 0;
}

static int cmd_info(char *args) { 
  char *arg = strtok(NULL, " ");
  if(arg == NULL) {
    printf("%s - %s\n", cmd_table[CMD_INFO_IDX].name,
           cmd_table[CMD_INFO_IDX].description);
  } else if(strcmp(arg, "r") == 0) {
    isa_reg_display();
  } else if(strcmp(arg, "w") == 0) {
    print_wps();
  }
  return 0;
}

// cmd_x的用法是x N(base 10 integer) addr(expr) Print [N] bytes of memory from [addr]
// 实现中主要的麻烦是换行控制
static int cmd_x(char *args) { 
  i32 ret;
  char *arg = strtok(NULL, " ");
  if (arg == NULL) {
    printf("%s - %s\n", cmd_table[CMD_X_IDX].name,
          cmd_table[CMD_X_IDX].description);
    return 0;
  } 
    // 第一个参数N只能是10进制
  errno = 0;
  int N = strtol(arg, NULL, 10);
  if (N <= 0 || errno!=0) {
    printf("invalid N\n");
    return 0;
  }
  arg = strtok(NULL, " ");
  vaddr_t addr;
  ret = expr(arg, &addr);
	if(ret<0) return 0;
  if (!is_valid_addr(addr)) {
    printf("Cannot access memory at address at 0x%x\n", addr);
    return 0;
  }
  uint32_t tmp;
  int i = 0;
	const int byte_per_line = 8; 
  for (; i < N; i++) {
    // 没检查内存是否有效
    if (addr >= PMEM_SIZE) {
      ++i; // 为啥要++i, 为了与下面putchar('\n')的逻辑一致
      break;
    }
    tmp = vaddr_read(addr, 1);
    if(i%byte_per_line==0) {
      // 分出分支是因为起始需要输出地址
      printf("0x%.8x: 0x%.2x", addr, tmp);
    } else {
      printf(" 0x%.2x%c", tmp, i % byte_per_line == (byte_per_line-1) ? '\n' :'\0');
    }
    ++addr;
  }
  if(i%byte_per_line!=0) putchar('\n');
  return 0;
}

void ui_mainloop(int is_batch_mode) {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  for (char *str; (str = rl_gets()) != NULL;) {  //? 什么时候有可能得到NULL, 我怎么感觉根本没这个机会?
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) {
      continue;
    }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1; // 指向字符串str中args的位置, 其实用strtok_r会不会好一些
    if (args >= str_end) {
      args = NULL;
    }
    // 因为一些原因不明的bug, 这个args完全不能用, 传参突然就发生了改变, 虽然各种函数都以它为参数, 可是事实上就是, 没什么用.
#ifdef HAS_IOE
    extern void sdl_clear_event_queue(void);
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) { // quit命令的实现方法
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
