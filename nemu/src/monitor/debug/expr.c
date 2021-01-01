#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <regex.h>
#include "common.h"
#define MAX_TOKEN_NUM 1000
#define OP_NUM 20 // OP_PRIORITY的默认大小
#define EVAL_ERROR(...) printf(__VA_ARGS__); *is_error = 1; return 0
#define OPERAND_TYPE(type)  (type == TK_DEC_NUMBER || type == TK_HEX_NUMBER || type == TK_REG) // 判断是否是操作数类型, 3种, 10进制数, 16进制数, 寄存器
#define UNARY_LEFT_OP(type) (type==TK_DEREF) // 判断是否是单目左边的运算符

// 需要解析以下类型
// 十进制整数
// +, -, *, /
// (, )
// 空格串(一个或多个空格)

/*TK_ENUM_START和TK_ENUM_END并不是实际的符号, 仅仅是为了确保初始化OP_PRIORITY数组时初始化了所有符号*/
/*!写法上要确保, 符号必须在TK_ENUM_START和TK_ENUM_END之间*/
enum {
  TK_NOTYPE = 256, TK_DEC_NUMBER, TK_HEX_NUMBER, TK_REG, TK_L_PAREN, TK_R_PAREN, TK_ASTERISK, TK_ENUM_START=0, TK_DEREF, TK_EQ, TK_NEQ, TK_OP_AND, TK_OP_PLUS, TK_OP_SUB, TK_OP_MUL, TK_OP_DIV
, TK_ENUM_END}; //? TK_NOTYPE为什么是256? 为什么不是0? 有什么用意?

/* 优先级越高越大, 很不好的是, 目前硬编码了 */
int OP_PRIORITY[OP_NUM];
// 对于这个数组的访问, 是通过OP_PRIORITY[type]这样来的

static struct rule {
  char *regex;
  int token_type;
} rules[] = {
    {" +", TK_NOTYPE}, // spaces //? 但是没有\t这些
    {"==", TK_EQ},     // equal
    {"!=", TK_NEQ},
    {"&&", TK_OP_AND},
    {"0x[[:xdigit:]]+", TK_HEX_NUMBER}, // 必须放在10进制之前
    {"[[:digit:]]+", TK_DEC_NUMBER},
    {"\\+", TK_OP_PLUS},
    {"-", TK_OP_SUB},
    {"\\*", TK_ASTERISK},
    {"/", TK_OP_DIV},
    {"\\(", TK_L_PAREN},
    {"\\)", TK_R_PAREN},
    {"\\$(eax|ecx|edx|ebx|esp|ebp|esi|edi|ax|cx|dx|bx|sp|bp|si|di|al|cl|dl|bl|"
     "ah|ch|dh|bh)",TK_REG}
};
//  测试正则表达式的文件: /Users/quebec/Playground/example/c/regex.c

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]) )
#define INF 1000

static regex_t re[NR_REGEX] = {};

/**
 * 初始化符号优先级数组OP_PRIORITY, 由于算法关系, 不需要考虑括号的优先级, 只需要考虑运算符的优先级设定
 * 数值越小, 表示优先级越低, 比如==的优先级是0.
 * 具体数值, 参考<https://en.cppreference.com/w/c/language/operator_precedence>
 * 为什么是负数, 这主要是因为之前的逻辑是越小优先级越低
 */

void init_priotity() {
  OP_PRIORITY[TK_OP_AND] = -11;
  OP_PRIORITY[TK_EQ]=-7;
  OP_PRIORITY[TK_NEQ] = -7;
  OP_PRIORITY[TK_OP_PLUS] = -4;
  OP_PRIORITY[TK_OP_SUB] = -4;
  OP_PRIORITY[TK_OP_MUL] = -3;
  OP_PRIORITY[TK_OP_DIV] = -3;
  OP_PRIORITY[TK_DEREF] = -2;
  for (int i = TK_ENUM_START+1; i < TK_ENUM_END; i++) {
    Assert(OP_PRIORITY[i]!=0,"Precedence of operator %d is not initialized", i);
  }
}

/**
 * 这里除了初始化re数组(regex数组), 还初始化了priority, 没有解耦合, 但这是为了减轻写测试的负担
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
  init_priotity();
}

typedef struct token {
  int type;
  char str[32];
} Token;

static Token tokens[MAX_TOKEN_NUM] __attribute__((used)) = {};
static int len_tokens __attribute__((used))  = 0;  // 记录符号正确解析时, tokens的长度, 但是能不用尽量不用. 在make_tokens正确解析时, 会被设置为长度, 在make_token解析错误时, 设为0.

/**
 * 根据字符串e从0开始填写tokens
 * 在make_tokens正确解析时, 会被设置为长度, 在make_token解析错误时, 设为0
 * 如果解析错误, 返回0, 否则返回tokens的个数
 * 为了make_token实现上的简单, 对于寄存器, 去除0x不是它的工作, 是相应的函数的工作, 也就是isa_reg_str2val的工作
 * 解引用还是乘号的确定, 我们不在这里做, 因为我认为make_token的作用仅仅是解析正则符号, 弄懂是什么意思, 这是预处理的工作, 也就是expr
 */
uint8_t make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  int nr_token = 0;
  len_tokens = 0; // 如果不这样写, return false的时候都需要设置为0. 这样写以后, 只有return true的时候才需要设置
  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) { // 优先级的来源
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) { // 不存在子组需要match, 而且pmatch.rm_so == 0确定必须是从开始匹配, 但能不能用^呢?
        // switch的结果无论如何, nr_token都是会++的, 因此我们可以在这里判断
        if (nr_token >= MAX_TOKEN_NUM) {
          printf("The expr is too long, we cannot deal with it\n");
          return false;
        }
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        // Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
        //     i, rules[i].regex, position, substr_len, substr_len, substr_start);
        position += substr_len;
        // 此时nr_token指向的是未指向的位置
        tokens[nr_token].type = rules[i].token_type;
        if (OPERAND_TYPE(rules[i].token_type)) {
          if (substr_len > 31) { // 太长了, 处理不了
            printf("too large number, sorry, we cannot deal with it: %.*s",
                   substr_len, substr_start);
            return false;
          } else {
            memcpy(tokens[nr_token].str, substr_start, substr_len);
            tokens[nr_token].str[substr_len] = '\0';
          }
        }
        nr_token += 1;

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

/**
 * pre_tokens是预处理后的数组, left和right是闭区间, is_error是否有错误, 如果为1, 发生了错误, 比如括号不匹配
 * 返回解析得到的值
 * 不保证解析结果错误会怎样, 但不能让程序崩
*/
int32_t eval(Token* pre_tokens, int left, int right, bool* is_error) {
  // is_error必须主动是0, 因为不能指望调用者将它初始化了
  // 保证priority被初始化了
  Assert(OP_PRIORITY[TK_OP_AND] != 0, "function <init_priotity> hasn't been called");
  *is_error = 0;
  // 由于是expr调用的, 越界是不可能发生的
  if(left==right) {
    switch (pre_tokens[left].type) { 
      case TK_DEC_NUMBER: {
        return atoi(pre_tokens[left].str);
      }
      case TK_HEX_NUMBER: {
        return strtol(pre_tokens[left].str, NULL, 16);
      }
      case TK_REG: {
        bool success;
        uint32_t reg_content = isa_reg_str2val(pre_tokens[left].str, &success);
        if(success) {
          return reg_content;
        } else {
          // 这不是用户的问题, 是代码写错了, 不可能出现这种情况
          Assert("regname recognization is incorrect. Since we only recognize eax these, now isa_reg_str2val tells me he cannot find %s.\n", pre_tokens[left].str);
        }
        break;
      }
      default: {
        EVAL_ERROR("%s is not a number, its type is %d\n", pre_tokens[left].str, pre_tokens[left].type);
      }
    }
  }
  // bug: 对于是不是被一堆括号包围的情况, 没有正确给出判断,  因为没有检查匹配, 比如(-52)*(-58)就不行
  // if (pre_tokens[left].type==TK_L_PAREN && pre_tokens[right].type==TK_R_PAREN) {
  //   return eval(pre_tokens, left+1, right-1, is_error);
  // }

  // 合并检查括号和扫描主符号, 由于检查是不是被一对匹配的括号包围, 因此也在这里做判断
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
      // 对于括号是不是不匹配的判断
      case TK_R_PAREN: {
        lp_num--;
        if (lp_num < 0) {
          EVAL_ERROR("parenthesis not matches\n");
        }
        break;
      }
      // 这里应该只是优先级判断
      case TK_REG:
      case TK_HEX_NUMBER:
      case TK_DEC_NUMBER: break;
      default: { // default是所有的符号的情况, 当前类型保证了这一点, 之后不知道
                 // 不在括号中和是运算符, 不满足不可能
        if (lp_num==0&&OP_PRIORITY[tmp]<=cur_priority) { // 不能是<, 因为同级的选靠右的
          op_pos=i;
          cur_priority = OP_PRIORITY[tmp];
        }
        break;
      }
    }
  }
  if(lp_num!=0) {
    EVAL_ERROR("parenthesis not matches\n");
  }
  // 没有发现主符号的情况, 有可能是因为被一对括号包围
  if (op_pos==-1) {
    // 这种判断是否被一对括号包围的正确性在于, 如果()且不匹配, 那么必然有() (), 但这两个括号之间必须有运算符包围
    if(pre_tokens[left].type==TK_L_PAREN && pre_tokens[right].type==TK_R_PAREN) {
      return eval(pre_tokens, left + 1, right - 1, is_error);
    } else {
      EVAL_ERROR("cannot find operator\n");
    }
  }
  // 这样就应该扫描出了主符号
  int32_t res;
  if (!UNARY_LEFT_OP(pre_tokens[op_pos].type)) {
    // 如果不是单目运算符
    bool left_error, right_error;
    int32_t left_val = eval(pre_tokens, left, op_pos - 1, &left_error);
    if (left_error) {
      *is_error = 1;
      return 0;
    }
    int32_t right_val = eval(pre_tokens, op_pos + 1, right, &right_error);
    if (right_error) {
      *is_error = 1;
      return 0;
    }
    switch (pre_tokens[op_pos].type) {
      case TK_EQ: {
        res = left_val ==
              right_val; // 不好是同一个吧, 万一并行了咋办(虽然我也不知道并不并行)
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
        if (right_val == 0) {
          EVAL_ERROR("divide 0\n");
        }
        res = left_val / right_val;
        break;
      }
      case TK_OP_AND: {
        res = left_val && right_val;
        break;
      }
      case TK_NEQ: {
        res = left_val != right_val;
        break;
      }
    }
  } else {
    // 如果是*那么必然是第一个
    Assert(left==op_pos, "deference * is not the first");
    bool right_error;
    int32_t right_val = eval(pre_tokens, op_pos + 1, right, &right_error);
    if (is_valid_addr(right_val)) {
      res = vaddr_read(right_val, 4);
    } else {
      EVAL_ERROR("not a valid address");
    }
  }
  return res;
}
/**
 * e是表达式字符串, 返回所求值(uint32_t)
*/
uint32_t expr(char *e, bool *success) {
  // 调用eval之前, 主要是预处理, 包括去空格, 对负数的处理, 确定*是乘号还是解引用
  if (!make_token(e)) {
    *success = false;
    return 0;
  }
  // 对tokens数组进行预处理. 去空格, 加上0
  Token pre_tokens[MAX_TOKEN_NUM<<1]; // 应该比tokens数组更大,
                                   // 因为可能出现很多负数, 最坏的情况,
                                   // 全是负数, 那么占1/3, 因此数组为43
  int j = 0; // j是pre_tokens的索引, 存的是第一个未赋值的
  for(int i = 0; i < len_tokens; i++) {
    switch(tokens[i].type) {
      case TK_NOTYPE:
        break;
      case TK_OP_SUB:{
        // 那么需要做一番判断
        if (j==0 || !(pre_tokens[j - 1].type == TK_R_PAREN || OPERAND_TYPE(pre_tokens[j - 1].type))) { // 负数的情况
          // 需要追加0
          pre_tokens[j].type = TK_DEC_NUMBER;
          pre_tokens[j].str[0] = '0';
          pre_tokens[j].str[1] = '\0';
          ++j;
        }
        pre_tokens[j++] = tokens[i];
        break;
      }
      case TK_ASTERISK: {
        pre_tokens[j].type = (j==0 || !(pre_tokens[j - 1].type == TK_R_PAREN || OPERAND_TYPE(pre_tokens[j - 1].type)))?TK_DEREF:TK_OP_MUL;
        ++j;
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
