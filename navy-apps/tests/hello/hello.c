#include <unistd.h>
#include <stdio.h>

int main(int argc, char *argv[], char *envp[]) {
	printf("[main]argc:%d, argv:%x, envp:%x, argv[0](%x):%s, argv[1](%x):%s\n", argc, argv, envp, argv[0], argv[0], argv[1], argv[1]);
  write(1, "Hello World!\n", 13);
  // int i = 2;
  // volatile int j = 0;
  // while (1) {
  //   j ++;
  //   if (j == 10000) {
  //     printf("Hello World from Navy-apps for the %dth time!\n", i ++);
  //     j = 0;
  //   }
  // }
  return 0;
}
