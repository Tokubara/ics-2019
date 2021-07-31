#include "proc.h"

#define MAX_NR_PROC 4

static PCB pcb[MAX_NR_PROC] __attribute__((used)) = {};
static PCB pcb_boot = {};
PCB *current = NULL;

void switch_boot_pcb() {
  current = &pcb_boot;
}

void hello_fun(void *arg) {
  int j = 1;
  while (1) {
    // Log("Hello World from Nanos-lite for the %dth time!", j);
    // printf("Hello World from %s\n", (char*)arg);
    j ++;
    _yield();
  }
}

void init_proc() {
  // context_kload(&pcb[0], hello_fun, "China");
  char* argv_0[1];
  argv_0[0] = NULL;
  char* envp_0[1];
  envp_0[0] = NULL;
  context_uload(&pcb[0], "/bin/dummy", argv_0, envp_0);
  char* argv_1[3];
  argv_1[0] = "/bin/dummy";
  argv_1[1] = "--skip";
  argv_1[2] = NULL;
  // char* argv[1];
  // argv[0] = NULL;
  char* envp_1[1];
  envp_1[0] = NULL;
  context_uload(&pcb[1], "/bin/bmptest", argv_1, envp_1);
  switch_boot_pcb();

  Log("Initializing processes...");

  // load program here
  // naive_uload(&pcb[0], "/bin/dummy");
}

bool is_kernel_thread(_Context* c);

_Context* schedule(_Context *prev) {
  // save the context pointer
  current->cp = prev; //? 为什么需要这一句?不用于恢复, 不就没用么

  current = (current == &pcb[0] ? &pcb[1] : &pcb[0]);
  if (is_kernel_thread(current->cp)) {
    set_tss_esp0(0);
  } else {
    set_tss_esp0((size_t)current->stack + STACK_SIZE);
  }

  // then return the new context
  return current->cp;
}
