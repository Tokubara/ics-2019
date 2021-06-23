#include "nemu.h"
#include "monitor/diff-test.h"

bool isa_difftest_checkregs(CPU_state *ref_r, vaddr_t pc) {
  return memcmp(&cpu,ref_r,sizeof(cpu.gpr[0])*8)==0;
}

void isa_difftest_attach(void) {
}
