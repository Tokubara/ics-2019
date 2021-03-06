#ifndef __PROC_H__
#define __PROC_H__

#include "common.h"
#include "memory.h"

#define STACK_SIZE (8 * PGSIZE)

typedef enum task_status {
  RUNNING,
  EXITED
} task_status;

typedef union {
  uint8_t stack[STACK_SIZE] PG_ALIGN;
  struct {
    _Context *cp;
    _AddressSpace as;
    // we do not free memory, so use `max_brk' to determine when to call _map()
    uintptr_t max_brk;
    task_status status;
    unsigned priority;
    unsigned ticks;
    unsigned pid;
  };
} PCB;

extern PCB pcb[];
extern PCB *current;
extern PCB *fg_pcb;

#endif
