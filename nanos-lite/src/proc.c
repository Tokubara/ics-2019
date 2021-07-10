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
    printf("Hello World from %s\n", (char*)arg);
    j ++;
    _yield();
  }
}

void init_proc() {
  context_kload(&pcb[0], hello_fun, "China");
  context_kload(&pcb[1], hello_fun, "universe");
  switch_boot_pcb();

  // Log("Initializing processes...");

  // load program here
  // naive_uload(NULL, "/bin/dummy");
}

_Context* schedule(_Context *prev) {
  // save the context pointer
  current->cp = prev; //? 为什么需要这一句?不用于恢复, 不就没用么

  current = (current == &pcb[0] ? &pcb[1] : &pcb[0]);

  // then return the new context
  return current->cp;
}
