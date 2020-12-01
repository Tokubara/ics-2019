#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <string.h>
#include <sys/types.h>
#include <regex.h>

// 需要解析以下类型
// 十进制整数
// +, -, *, /
// (, )
// 空格串(一个或多个空格)

enum {
  TK_NOTYPE = 256, TK_EQ, TK_DEC_NUMBER, TK_OP_PLUS, TK_OP_SUB, TK_OP_MUL, TK_OP_DIV, TK_L_PAREN, TK_R_PAREN
}; //? TK_NOTYPE为什么是256? 为什么不是0? 有什么用意?

static struct rule {
  char *regex;
  int token_type;
} rules[] = {

    /* TODO: Add more rules.
     * Pay attention to the precedence level of different rules.
     */

    {" +", TK_NOTYPE}, // spaces //? 但是没有\t这些
    {"==", TK_EQ},     // equal
    {"[[:digit:]]+", TK_DEC_NUMBER},  // 已测试
    {"\\+", TK_OP_PLUS}, // plus  //? 这里为什么会是'+', 而不是一个enum type? 已测试
    {"-", TK_OP_SUB}, // 已测试
    {"\\*", TK_OP_MUL}, // 不用测试, 与+类似
    {"/", TK_OP_DIV},  // 已测试
    {"\\(", TK_L_PAREN}, // 已测试
    {"\\)", TK_R_PAREN}, // 不用测试
};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]) )

static regex_t re[NR_REGEX] = {};

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[32];
} Token;

static Token tokens[32] __attribute__((used)) = {};
static int nr_token __attribute__((used))  = 0;

bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) { // 尝试每一个, 这是优先级的来源
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) { // 不存在子组需要match, 而且pmatch.rm_so == 0确定必须是从开始匹配, 但能不能用^呢?
        // switch的结果无论如何, nr_token都是会++的, 因此我们可以在这里判断
        if (nr_token >= 32) {
          printf("The expr is too long, we cannot deal with it\n");
          return false;
        }
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);
        position += substr_len;
        switch (rules[i].token_type) {
          case TK_DEC_NUMBER: {
            if (substr_len>31) { // 太长了, 处理不了
              printf("too large number, sorry, we cannot deal with it: %.*s", substr_len, substr_start);
              return false;
            }
            // 否则就是把一片字符串复制过来. 而且需要追加\0, 原来的字符串是没有\0的, 因此strcpy是不行的, 用memcpy
            memcpy(tokens[nr_token].str, substr_start, substr_len);
            tokens[nr_token].str[substr_len]='\0';
            nr_token+=1;
            break;
          }
          default: {
            tokens[nr_token++].type = rules[i].token_type;
            break;
          };  // 我想默认应该是直接赋值
        }
        break;  // 离开for循环, 因为已经找到了
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}

uint32_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }
  // TODO: 对token数组进行预处理. 去空格, 加上0
  // int is_error=0;
  // uint32_t expr_val = eval(0, len, &is_error);

  return 0;
}
