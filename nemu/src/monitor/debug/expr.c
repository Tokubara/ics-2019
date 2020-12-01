#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <regex.h>

// 需要解析以下类型
// 十进制整数
// +, -, *, /
// (, )
// 空格串(一个或多个空格)

/*写法上, 只有符号才从0开始*/
enum {
  TK_NOTYPE = 256, TK_DEC_NUMBER, TK_L_PAREN, TK_R_PAREN, TK_EQ=0, TK_OP_PLUS, TK_OP_SUB, TK_OP_MUL, TK_OP_DIV
}; //? TK_NOTYPE为什么是256? 为什么不是0? 有什么用意?

static struct rule {
  char *regex;
  int token_type;
} rules[] = {
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
#define INF 1000

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
static int len_tokens __attribute__((used))  = 0;  // 记录符号正确解析时, tokens的长度, 但是能不用尽量不用. 在make_tokens正确解析时, 会被设置为长度, 在make_token解析错误时, 设为0.

/**
 * 根据字符串e从0开始填写tokens
 * 在make_tokens正确解析时, 会被设置为长度, 在make_token解析错误时, 设为0
 * 如果解析错误, 返回0, 否则返回tokens的个数
 */
uint8_t make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  int nr_token = 0;
  len_tokens = 0; // 如果不这样写, return false的时候都需要设置为0. 这样写以后, 只有return true的时候才需要设置
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

        // Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
        //     i, rules[i].regex, position, substr_len, substr_len, substr_start);
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
  // 此时正确解析
  len_tokens = nr_token;
  return nr_token;
}

// TK_EQ=0, TK_OP_PLUS, TK_OP_SUB, TK_OP_MUL, TK_OP_DIV,
/* 优先级越高越大, 很不好的是, 目前硬编码了 */
const int OP_PRIORITY[]= {
  0, 1, 1, 2, 2
};

/**
 * pre_tokens是预处理后的数组, left和right是闭区间, is_error是否有错误, 如果为1, 发生了错误, 比如括号不匹配
 * 返回解析得到的值
 * 不保证解析结果错误会怎样, 但不能让程序崩
*/
uint32_t eval(Token* pre_tokens, int left, int right, bool* is_error) {
  // is_error必须主动是0, 因为不能指望调用者将它初始化了
  *is_error=0;
  // 由于是expr调用的, 越界是不可能发生的
  if(left==right) {
    if(pre_tokens[left].type==TK_DEC_NUMBER) {
      return atoi(pre_tokens[left].str);
    } else {
      *is_error=1;
      return 0;  // 要是有能直接return到原来的函数的办法就好了
    }
  }
  if (pre_tokens[left].type==TK_L_PAREN && pre_tokens[right].type==TK_R_PAREN) {
    return eval(pre_tokens, left+1, right-1, is_error);
  }

  // 似乎可以合并检查括号和扫描主符号
  int cur_priority=INF; // 这样只要有符号就能比它小
  int op_pos = -1;
  int lp_num = 0;
  int tmp;
  for(int i = left; i <= right; i++) {
    switch ((tmp=pre_tokens[i].type)) {
      case TK_L_PAREN: {
        lp_num++;
        break;
      }
      case TK_R_PAREN: {
        lp_num--;
        if (lp_num < 0) {
          // 异常
          *is_error = 1;
          return 0;
        }
        break;
      }
      // 这里应该只是优先级判断
      case TK_DEC_NUMBER: break;
      default: { // default是所有的符号的情况
        if (OP_PRIORITY[tmp]<=cur_priority) { // 不能是<, 因为同级的选靠右的
          op_pos=i;
          cur_priority = OP_PRIORITY[tmp];
        }
        break;
      }
    }
  }
  // 没有发现主符号的情况
  if (op_pos==-1) {
    *is_error=1;
    return 0;
  }
  // 这样就应该扫描出了主符号
  uint32_t res;
  bool left_error=0, right_error=0;
  uint32_t left_val = eval(pre_tokens, left, op_pos - 1, &left_error);
  if (left_error) {
    *is_error = 1;
    return 0;
  }
  uint32_t right_val = eval(pre_tokens, op_pos + 1, right, &right_error);
  if (right_error) {
    *is_error = 1;
    return 0;
  }
  switch (pre_tokens[op_pos].type) {
    case TK_EQ: {
      res = left_val == right_val; // 不好是同一个吧, 万一并行了咋办(虽然我也不知道并不并行)
      break;
    }
    case TK_OP_PLUS: {
      res = left_val + right_val;
      break;
    }
    case TK_OP_SUB: {
      res = left_val - right_val;
      break;
    }
    case TK_OP_MUL: {
      res = left_val * right_val;
      break;
    }
    case TK_OP_DIV: {
      res = left_val / right_val;
      break;
    }
  }
  return res;
}

uint32_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }
  // 对tokens数组进行预处理. 去空格, 加上0
  Token pre_tokens[43]; // 应该比tokens数组更大, 因为可能出现很多负数, 最坏的情况, 全是负数, 那么占1/3, 因此数组为43
  int j = 0; // j是pre_tokens的索引, 存的是第一个未赋值的
  for(int i = 0; i < len_tokens; i++) {
    switch(tokens[i].type) {
      case TK_NOTYPE:
        break;
      case TK_OP_SUB:{
        // 那么需要做一番判断
        if (j==0 || (pre_tokens[j - 1].type != TK_R_PAREN && pre_tokens[j - 1].type != TK_DEC_NUMBER)) { // 负数的情况
          // 需要追加0
          pre_tokens[j].type = TK_DEC_NUMBER;
          pre_tokens[j].str[0] = '0';
          pre_tokens[j].str[1] = '\0';
          ++j;
        }
        pre_tokens[j++] = tokens[i]; // 这一句话与下面一句话都可以不要
        break;
      }
      default:{
        pre_tokens[j++]=tokens[i];
        break;
      }
    }
  }
  // 调用eval
  bool is_error=0;
  // char* err_msg;
  uint32_t expr_val = eval(pre_tokens, 0, j - 1, &is_error); // 这里是闭区间
  // return (is_error>0?)
  *success=is_error>0?0:1;
  return expr_val;
  // return 0;
}
