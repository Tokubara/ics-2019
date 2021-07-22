#include "memory.h"

// 这好像意味着空闲物理地址的起始
static void *pf = NULL;

// 分配nr_page这么多物理页, 返回物理地址, 其实就是pf+PGSIZE * nr_page
void* new_page(size_t nr_page) {
  void *p = pf;
  pf += PGSIZE * nr_page;
  assert(pf < (void *)_heap.end);
  return p;
}

void free_page(void *p) {
  panic("not implement yet");
}

/* The brk() system call handler. */
int mm_brk(_AddressSpace *as, uintptr_t brk, uintptr_t new_brk) {
  Log_debug("old_brk:%x, new_brk:%x", brk, new_brk);
  if(new_brk > brk) {
    add_vmap_range(as, brk, new_brk); // 未考虑错误情况
  }
  return 0;
}

void init_mm() {
  pf = (void *)PGROUNDUP((uintptr_t)_heap.start);
  Log("free physical pages starting from %p", pf);

  _vme_init(new_page, free_page);
}
