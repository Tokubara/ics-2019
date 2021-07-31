#include "rtl/rtl.h"

void raise_intr(uint32_t NO, vaddr_t ret_addr) {
  // 压栈
  rtl_push(&cpu.eflags.val);
  rtl_push(&cpu.cs);
  rtl_push(&ret_addr); // 本来写的是压cpu.pc, 但是应该不是, 应该是ret_addr, 因为cpu.pc还没有指向下一条指令, 还是指向int指令本身的
  vaddr_t int_addr;
  rtl_desc_addr(&int_addr, NO, 1);
  rtl_j(int_addr); // 不确定这样跳转是否合适
}

bool isa_query_intr(void) {
  return false;
}
