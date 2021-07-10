#include <stdlib.h>
#include <assert.h>

int main(int argc, char *argv[], char *envp[]);
extern char **environ;


void _start() {
  unsigned esp;
  asm volatile ("movl %%eax, %0;movl %%eax, %%esp;subl 4, %%esp":"=r"(esp));
  
  int argc = esp;
  char** argv = esp+4;
  char** envp = esp+8;
	printf("[start]gc:%d, argv:%x, envp:%x, argv[0](%x):%s, argv[1](%x):%s\n", argc, argv, envp, argv[0], argv[0], argv[1], argv[1]);
  // char *env[] = {NULL};
  // environ = env;
  int ret = main(argc, argv, envp);
  exit(ret);
  assert(0);
}
