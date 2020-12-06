#include "common.h"
int init_monitor(int, char *[]);

void ui_mainloop(int);

void init_regex();
// uint8_t make_token(char *);
uint32_t expr(char *, bool *);

// 原来的
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

// 用于debug写死的字符串
int main() {
  init_regex();
  char buf[10000] = "($eax+0x16)&&($ax!=0x08)"; // 测试表达式写在这里就行
  uint8_t success = 0;
  uint32_t res = expr(buf, &success);
  if (success) {
    printf("%u\n", res);
    } else {
      printf("error");
    }
  return 0;
}
// 用于不断读取
// #include <stdio.h>

// int main() {
//   init_regex();
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