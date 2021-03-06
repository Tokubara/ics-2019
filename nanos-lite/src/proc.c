#include "proc.h"

#define MAX_NR_PROC 4

PCB pcb[MAX_NR_PROC] __attribute__((used)) = {};
static PCB pcb_boot = {};
PCB *current = NULL;
PCB *fg_pcb = NULL;


void switch_boot_pcb() {
  current = &pcb_boot;
}

void hello_fun(void *arg) {
  int j = 1;
  while (1) {
    // Log("Hello World from Nanos-lite for the %dth time!", j);
    printf("Hello World from %s for the %dth time\n", (char*)arg, j);
    j++;
    _yield();
  }
}

void init_proc() {
#ifdef DISPLAY
  context_kload(&pcb[0], hello_fun, "China", 1, 0);

  char* argv_1[1];
  argv_1[0] = NULL;
  char* envp_1[1];
  envp_1[0] = NULL;
  context_uload(&pcb[1], "/bin/init", argv_1, envp_1, 300, 1);

  char* argv_2[1];
  argv_2[0] = NULL;
  char* envp_2[1];
  envp_2[0] = NULL;
  context_uload(&pcb[2], "/bin/init", argv_2, envp_2, 300, 2);

  char* argv_3[1];
  argv_3[0] = NULL;
  char* envp_3[1];
  envp_3[0] = NULL;
  context_uload(&pcb[3], "/bin/init", argv_3, envp_3, 300, 3);

  fg_pcb = &pcb[1]; // 默认是pcb[1]
  Log_trace("fg_pcb cp(esp):%x, cr3:%x", (size_t)fg_pcb->cp, fg_pcb->as.ptr);
#else
  context_kload(&pcb[0], hello_fun, "China", 1, 0);

  char* argv_1[1];
  argv_1[0] = NULL;
  char* envp_1[1];
  envp_1[0] = NULL;
  context_uload(&pcb[1], "/bin/dummy", argv_1, envp_1, 300, 1);
#endif


  Log("Initializing processes...");
}

bool is_kernel_thread(_Context* c);
int check_function_key();

_Context* schedule(_Context *prev) {
  // save the context pointer
  current->cp = prev; // 需要这一句是因为pcb数组的那个pcb需要
  Log_trace("old cp(esp):%x, cr3:%x", (size_t)current->cp, current->as.ptr);
#ifdef DISPLAY
  // 先看看之前运行的是不是hello
  if (current == &pcb[0]) {
    Log_trace("switch to fg_pcb");
    current = fg_pcb;
  } else {
    Log_trace("switch to hello");
    current = &pcb[0];
  }

#else
  // 如果不是展示, 那就是顺序切换
  static size_t next_index = 0;
  // 得到next_index
  int i;
  for (i = 0; (pcb[next_index].cp == NULL || pcb[next_index].status == EXITED) && i < MAX_NR_PROC; ++i) {
    next_index = (next_index + 1) % MAX_NR_PROC;
  }
  if (i == MAX_NR_PROC) {
    Log_info("All done!!");
    _halt(0);
  }
  Log_trace("next pcb index=%u", next_index);
  current = &pcb[next_index];
  next_index = (next_index + 1) % MAX_NR_PROC;
#endif
  Log_trace("new cp(esp):%x, cr3:%x", (size_t)current->cp, current->as.ptr);
  if (is_kernel_thread(current->cp)) {
    set_tss_esp0(0);
  } else {
    set_tss_esp0((size_t)current->stack + STACK_SIZE);
  }

  // then return the new context
  return current->cp;
}
