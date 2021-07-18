#include <stdlib.h>
#include <assert.h>

int main(int argc, char *argv[], char *envp[]);
// int main();
extern char **environ;


void _start() {
  unsigned* esp;
  asm volatile ("popl %%ebx; popl %%esi; popl %%edi; movl %%eax, %0; movl %%eax, %%esp; subl 4, %%esp; pushl %%edi; pushl %%esi; pushl %%ebx":"=r"(esp));
  
  int argc = esp[0];
  char** argv = esp[1];
  char** envp = esp[2];
	// printf("[start]argc:%d, argv:%x, envp:%x, argv[0](%x):%s, argv[1](%x):%s\n", argc, argv, envp, argv[0], argv[0], argv[1], argv[1]);
  char *env[] = {NULL};
  environ = env;
  int ret = main(argc, argv, envp);
  // int ret = main();
  exit(ret);
  assert(0);
}
