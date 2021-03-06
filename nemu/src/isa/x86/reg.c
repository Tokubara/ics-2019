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
  cpu.pc = pc_sample; // pc被设为了随机数

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

// 打印所有寄存器, 以16进制和10进制打印
void isa_reg_display() {
  for (int i = 0; i < 8; i++) {
    printf("%s  0x%.8x %u\n", reg_name(i, 4), cpu.gpr[i]._32,
           cpu.gpr[i]._32);
  }
}
/**
 * s长这样:$eax,$ax,$al,$ah
 * 对这个函数的实现思路是: 如果首字母是e, 在数组regsl中查找, 如果最后一个字母是h或者l在第3个数组中查找
 * 值设置在val中, 解析成功, 即有效, 返回0, 否则返回-1
 * */
i32 isa_reg_str2val(const char *s, u32* val) {
  i32 ret = -1;
  if(s[1]=='e') {
    for (int i = 0; i < 8; i++) {
      if (strcmp(s+1, regsl[i]) == 0) {
        *val = reg_l(i);
        ret = 0;
      }
    }
  } else if(s[2]=='l' || s[2] == 'h') {
    for (int i = 0; i < 8; i++) {
      if (strcmp(s+1, regsb[i])==0) {
        *val = reg_b(i);
        ret = 0;
      }
    }
  } else {
    for (int i = 0; i < 8; i++) {
      if (strcmp(s+1, regsw[i])==0) {
        *val = reg_w(i);
        ret = 0;
      }
    }
  }
  return ret;
}
