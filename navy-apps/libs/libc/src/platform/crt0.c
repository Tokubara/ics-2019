#include <stdlib.h>
#include <assert.h>

int main(int argc, char *argv[], char *envp[]);
extern char **environ;

void _start(int argc, char *argv[], char *envp[]) {
  char *env[] = {NULL};
  environ = env;
  int ret = main(argc, argv, env);
  exit(ret);
  assert(0);
}
