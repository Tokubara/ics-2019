#include "rtl/rtl.h"
// #include "cpu/decode.h"

#define IRQ_TIMER 32

void raise_intr(uint32_t NO, vaddr_t ret_addr) {
  // 与实际CPU的行为不一致, 本来需要特权级检查, 如果没有特权级变化, 不会压入esp, ss, 由于没有特权级, 因此压入0来表示不变化
  rtl_lm_ph(&s0, &cpu.tss_esp0_paddr, 4);
  if (s0 == 0 || cpu.esp <= s0) { // s1存要压入的esp, s0存esp0. <=s0表示当前已在内核栈中
    rtl_li(&s1, 0);
    Log_trace("cpu.pc=%x, ret_addr:%x, esp not change, esp=%x, cr3=%x", cpu.pc, ret_addr, cpu.esp, cpu.cr3.val);
  } else {
    rtl_mv(&s1, &cpu.esp);
    rtl_mv(&cpu.esp, &s0);
    Log_trace("cpu.pc=%x, ret_addr:%x, old esp=%x, new esp=%x", cpu.pc, ret_addr, s1, s0);
  }
  // 压栈
  rtl_push(&cpu.ss);
  rtl_push(&s1);
  rtl_push(&cpu.eflags.val);
  cpu.eflags.IF = 0;
  rtl_push(&cpu.cs);
  rtl_push(&ret_addr); // 本来写的是压cpu.pc, 但是应该不是, 应该是ret_addr, 因为cpu.pc还没有指向下一条指令, 还是指向int指令本身的
  rtl_gate_desc_addr(&s1, NO);
  rtl_j(s1); // 不确定这样跳转是否合适
}

// extern DecodeInfo decinfo; // 本来就会0初始化
bool isa_query_intr(void) {
  if (cpu.eflags.IF == 1 && cpu.INTR == true) {
    cpu.INTR = false;
    raise_intr(IRQ_TIMER, cpu.pc);
    return true;
  } 
  return false;
}
