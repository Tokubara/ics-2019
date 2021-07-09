#include <stdio.h>

int main() {
  FILE *fp = fopen("/dev/events", "r");
  int time = 0;
  printf("Start to receive events...\n");
  while (1) {
    char buf[256];
    char *p = buf, ch;
    while ((ch = fgetc(fp)) != -1) {
      *p ++ = ch;
      // printf("ch:%c,%d\n", ch, ch);
      if(ch == '\n') { // 遇到换行符, 拷贝结束
        *p = '\0';
        break;
      }
    }

    int is_time = buf[0] == 't'; // 判断是哪一种时间
    time += is_time;
    if (!is_time) {
      printf("receive event: %s", buf);
    }
    else if (time % 1024 == 0) {
      printf("receive time event for the %dth time: %s", time, buf);
    }
  }

  fclose(fp);
  return 0;
}

