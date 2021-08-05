#include <stdlib.h>
#include <assert.h>

int main(int argc, char *argv[], char *envp[]);
// int main();
extern char **environ;


void _start(int argc, char** argv, char** envp) {
  // printf("[start]argc:%d, argv:%x, envp:%x, argv[0](%x):%s, argv[1](%x):%s\n", argc, argv, envp, argv[0], argv[0], argv[1], argv[1]);
  char *env[] = {NULL};
  environ = env;
  int ret = main(argc, argv, envp);
  // int ret = main();
  exit(ret);
  assert(0);
}
