#include "rtl/rtl.h"

void raise_intr(uint32_t NO, vaddr_t ret_addr) {
  // 压栈
  rtl_push(&cpu.eflags);
  rtl_push(&cpu.cs);
  rtl_push(&ret_addr); // 本来写的是压cpu.pc, 但是应该不是, 应该是ret_addr, 因为cpu.pc还没有指向下一条指令, 还是指向int指令本身的
  // idt首地址
  vaddr_t entry_addr = cpu.idt + 8*NO;
  // 得到门描述符
  unsigned lo = vaddr_read(entry_addr, 4);
  unsigned hi = vaddr_read(entry_addr+4, 4);
  // 得到地址
  vaddr_t int_addr = (lo&0x0000ffff) | (hi&0xffff0000);
  rtl_j(int_addr); // 不确定这样跳转是否合适
}

bool isa_query_intr(void) {
  return false;
}
