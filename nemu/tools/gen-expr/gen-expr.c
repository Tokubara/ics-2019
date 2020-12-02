#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <string.h>

// this should be enough
static char buf[65536];
/**
 * 此函数会修改buf, start表示可以写入的字符串的位置, 返回表示可以开始写的字符串的位置
 * 由于调用的是sprintf, 可以保证以\0结尾
*/
static inline char* gen_rand_expr(char* start) {
  // buf[0] = '\0';
  // char* a = "12+23";
  // strcpy(buf, a);
  int n = rand();
  switch(n%3) {
    case 0: {
      // 写入一个数
      int m = rand();
      int len = sprintf(start, "%d", m);
      return (start+len);
    }
    case 1: {
      // 括号, 表达式的情况
      int len = sprintf(start, "(");
      char* new_start = gen_rand_expr(start+1);
      sprintf(new_start,")");
      return (new_start+1);
    }
    case 2: {
      // 表达式的情况
      char* new_start = gen_rand_expr(start);
      int m = rand();
      // int len;
      switch(m%5) {
        case 0: {
          sprintf(new_start, "==");
          new_start+=2;
          break;
        }
        case 1: {
          sprintf(new_start, "+");
          ++new_start;
          break;
        }
        case 2: {
          sprintf(new_start, "-");
          ++new_start;
          break;
        }
        case 3: {
          sprintf(new_start, "*");
          ++new_start;
          break;
        }
        case 4: {
          sprintf(new_start, "/");
          ++new_start;
          break;
        }
      }
      gen_rand_expr(new_start);
    }
  }

}

static char code_buf[65536];
static char *code_format =
"#include <stdio.h>\n"
"int main() { "
"  unsigned result = %s; "
"  printf(\"%%u\", result); "
"  return 0; "
"}";

int main(int argc, char *argv[]) {
  int seed = time(0);
  srand(seed);
  int loop = 1; // 生成表达式的个数
  if (argc > 1) { // 意味着可以空缺
    sscanf(argv[1], "%d", &loop); // 读入loop
  }
  int i;
  for (i = 0; i < loop; i ++) {
    gen_rand_expr(buf);
    sprintf(code_buf, code_format, buf);

    FILE *fp = fopen("/tmp/.code.c", "w"); //?奇怪的是, 我运行完这句话, 根本没有出现tmp这个文件夹
    assert(fp != NULL);
    fputs(code_buf, fp);
    fclose(fp);  // 把code_buf写入了

    int ret = system("gcc /tmp/.code.c -o /tmp/.expr");
    if (ret != 0) continue; // 出了错, 那么就没有意义了, 换一条

    fp = popen("/tmp/.expr", "r");
    assert(fp != NULL);

    int result;
    fscanf(fp, "%d", &result);
    pclose(fp);

    printf("%u %s\n", result, buf);
  }
  return 0;
}
