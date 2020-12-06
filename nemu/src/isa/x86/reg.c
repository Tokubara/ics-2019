#include "nemu.h"
#include <stdlib.h>
#include <time.h>

const char *regsl[] = {"eax", "ecx", "edx", "ebx", "esp", "ebp", "esi", "edi"};
const char *regsw[] = {"ax", "cx", "dx", "bx", "sp", "bp", "si", "di"};
const char *regsb[] = {"al", "cl", "dl", "bl", "ah", "ch", "dh", "bh"};

void reg_test() {
  srand(time(0));
  uint32_t sample[8];
  uint32_t pc_sample = rand();
  cpu.pc = pc_sample;

  int i;
  for (i = R_EAX; i <= R_EDI; i ++) {
    sample[i] = rand();
    reg_l(i) = sample[i];
    assert(reg_w(i) == (sample[i] & 0xffff));
  }

  assert(reg_b(R_AL) == (sample[R_EAX] & 0xff));
  assert(reg_b(R_AH) == ((sample[R_EAX] >> 8) & 0xff));
  assert(reg_b(R_BL) == (sample[R_EBX] & 0xff));
  assert(reg_b(R_BH) == ((sample[R_EBX] >> 8) & 0xff));
  assert(reg_b(R_CL) == (sample[R_ECX] & 0xff));
  assert(reg_b(R_CH) == ((sample[R_ECX] >> 8) & 0xff));
  assert(reg_b(R_DL) == (sample[R_EDX] & 0xff));
  assert(reg_b(R_DH) == ((sample[R_EDX] >> 8) & 0xff));

  assert(sample[R_EAX] == cpu.eax);
  assert(sample[R_ECX] == cpu.ecx);
  assert(sample[R_EDX] == cpu.edx);
  assert(sample[R_EBX] == cpu.ebx);
  assert(sample[R_ESP] == cpu.esp);
  assert(sample[R_EBP] == cpu.ebp);
  assert(sample[R_ESI] == cpu.esi);
  assert(sample[R_EDI] == cpu.edi);

  assert(pc_sample == cpu.pc);
}

void isa_reg_display() {
}
/**
 * s长这样:$eax
 * 对这个函数的实现思路是: 如果首字母是e, 在数组regsl中查找, 如果最后一个字母是h或者l在第3个数组中查找
 */
uint32_t isa_reg_str2val(const char *s, bool *success) {
  *success=1; // 默认成功, 失败再设置
  char regname[]={s[1],s[2],s[3],'\0'};
  if(regname[0]=='e') {
    for(int i = 0; i<8; i++) {
      if (strcmp(regname, regsl[i])) {
        return reg_l(i);
      }
    }
  } else if(regname[2]=='l' || regname[2] == 'h') {
    for (int i = 0; i < 8; i++) {
      if (strcmp(regname, regsb[i])) {
        return reg_b(i);
      }
    }
  } else {
    for (int i = 0; i < 8; i++) {
      if (strcmp(regname, regsw[i])) {
        return reg_w(i);
      }
    }
  }
  *success=0;
  return 0;
}
