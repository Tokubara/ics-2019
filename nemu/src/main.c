#include "common.h"
int init_monitor(int, char *[]);

void ui_mainloop(int);

void init_regex();
// uint8_t make_token(char *);
uint32_t expr(char *, bool *);

/** 非调试, 正常的nemu界面*/
// int main(int argc, char *argv[]) {
//   /* Initialize the monitor. */
//   int is_batch_mode = init_monitor(argc, argv);

//   /* Receive commands from user. */
//   ui_mainloop(is_batch_mode);

//   return 0;
// }

// // 不断从外界接受字符串测试
// #include <readline/readline.h>
// #include <stdlib.h>
// int main() {
//   init_regex();
//   // char buf[10000];

//   char *input;
//   while (true) {
//     // fgets(buf, 10000, stdin);
//     input = readline(">");
//     uint8_t success = 0;
//     uint32_t res = expr(input, &success);
//     if (success) {
//       printf("%u\n", res);
//     } else {
//       printf("error\n");
//     }
//     free(input);
//   }
//   return 0;
// }

/** 寄存器的测例 */
// static char *regsl[] = {"$eax", "$ecx", "$edx", "$ebx", "$esp", "$ebp", "$esi", "$edi"};
// static char *regsw[] = {"$ax", "$cx", "$dx", "$bx", "$sp", "$bp", "$si", "$di"};
// static char *regsb[] = {"$al", "$cl", "$dl", "$bl", "$ah", "$ch", "$dh", "$bh"};

// int main(int argc, char *argv[]) {
//   init_monitor(argc, argv);
//   uint8_t success = 0;
//   for(int i = 0; i < 8; i++) {
//     uint32_t reg32 = expr(regsl[i], &success);
//     uint32_t reg16 = expr(regsw[i], &success);
//     assert((reg32 & 0xffffu) == reg16);
//     if(i<4) {
//       uint32_t reg8_l = expr(regsb[i], &success);
//       uint32_t reg8_h = expr(regsb[i+4], &success);
      
//       assert((reg16 & 0xff00u)>>8 == reg8_h);
//       assert((reg16 & 0x00ffu) == reg8_l);
//     }
//   }
//   return 0;
// }

/** 对表达式的测例, 接受不断的输入 */
// #include <stdio.h>

// int main(int argc, char *argv[]) {
//   init_monitor(argc, argv);
//   // init_regex();
//   while (1) {
//     uint32_t std_ans;
//     char buf[65505];
//     int len = scanf("%u %s\n", &std_ans, buf);
//     if (len <= 0)
//       break;
//     uint8_t success = 0;
//     uint32_t res = expr(buf, &success);
//     if(res==std_ans) {
//       printf("right\n");
//     } else {
//       printf("%s std_ans:%u mine:%u\n", buf, std_ans ,res);
//     }
//   }
// }
extern uint8_t pmem[];
/** 固定表达式的测例 */
int main(int argc, char *argv[]) {
  init_monitor(argc, argv);
  // init_regex();
  // while (1) {
    // uint32_t std_ans;
    pmem[102]=9;
    pmem[103]=10;
    pmem[104]=17;
    pmem[105]=3;
    pmem[106]=4;
    pmem[107]=1;
    char buf[65505] = "(*102*7+*103)/*(100*(2==2)+4)";
    // int len = scanf("%u %s\n", &std_ans, buf);
    // if (len <= 0)
      // break;
    uint8_t success = 0;
    uint32_t res = expr(buf, &success);
    printf("%u\n", res);
    // if(res==std_ans) {
    //   printf("right\n");
    // } else {
    //   printf("%s std_ans:%u mine:%u\n", buf, std_ans ,res);
    // }
  // }
}