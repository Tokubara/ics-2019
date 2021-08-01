#include "cpu/exec.h"

CPU_state cpu;

rtlreg_t s0, s1, s2, s3, t0, t1, ir;

/* shared by all helper functions */
DecodeInfo decinfo; // 本来就会0初始化

void decinfo_set_jmp(bool is_jmp) {
  decinfo.is_jmp = is_jmp;
}

void isa_exec(vaddr_t *pc);

vaddr_t exec_once(void) {
  decinfo.seq_pc = cpu.pc;
  isa_exec(&decinfo.seq_pc);
  update_pc();

  bool isa_query_intr(void);
  isa_query_intr();

  return decinfo.seq_pc;
}
