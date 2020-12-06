#include <regex.h>
#include <readline/readline.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#define MAX_ERROR_MSG 0x1000

int main() {
  regex_t *r = malloc(sizeof(regex_t)); //分配编译后模式的存储空间
  while(1) {
    // 表示
    // regex_t *r;       //使用这句代替上句是不行的，因为指针未初始化
    char *text = readline("text>");                              //目标文本
    char *regtxt = readline("pattern>");                         //模式
    int status = regcomp(r, regtxt, REG_EXTENDED | REG_NEWLINE); //编译模式
    if (status) { //处理可能的错误
      char error_message[MAX_ERROR_MSG];
      regerror(status, r, error_message, MAX_ERROR_MSG);
      printf("Regex error compiling '%s': %s\n", text, error_message);
      return 1;
    }
    size_t nmatch = 1; //保存结果，每次匹配有两组$&，$1结果，分别是整体与子匹配
    regmatch_t m[nmatch];
    char *p = text;
    // while (1) {   
    // 之前在while中, 一个字符串可以匹配多次, 但由于需求, 这里只需要匹配1次                          //连续匹配直到行尾
    status = regexec(r, p, nmatch, m, 0); //匹配操作
    if (status) {                         //判断结束或错误
      char error_message[MAX_ERROR_MSG];
      regerror(status, r, error_message, MAX_ERROR_MSG);
      printf("Regex stop executing '%s': %s\n", text, error_message);
      return 1;
    }
    int i;
    for (i = 0; i < nmatch; i++) { //打印结果，注意regmatch_t中保存的偏移信息
      printf("%.*s, so = %d, eo = %d\n", m[i].rm_eo - m[i].rm_so,
              p + m[i].rm_so, m[i].rm_so, m[i].rm_eo);
    }
    // p += m[0].rm_eo;
    // }
    free(text);
    free(regtxt);
  } 
  regfree(r); //释放空间
  return 0;
}

