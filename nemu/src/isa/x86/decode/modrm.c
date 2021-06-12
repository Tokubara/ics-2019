#include "cpu/exec.h"

/**
 * 调用例子(仅仅在read_ModR_M被调用, 因此也是唯一调用方式): pc, &m(modr/m), rm(rm要写入信息的operand, 通常是decinfo中的dst)
 * 用于处理modr/m byte指示有一个操作数是地址的情况, read_ModR_M除了调用load_addr, 仅负责处理操作数是寄存器的情况
*/
// 在read_ModR_M已经处理了m->mod为3的情况, 这里处理的是另一种情况, 因此必有m->mod!=3, 而且, 指示的一定是一个内存地址(为3的时候指示的是一个寄存器). 所有的地址模式都可以用base index scale disp描述, 所以只要确定了这4个数, 即可确定内存地址.
void load_addr(vaddr_t *pc, ModR_M *m, Operand *rm) {
  assert(m->mod != 3);
  int32_t disp = 0;
  int disp_size = 4; // 默认是4位偏置, 在mod==1时会修改
  int base_reg = -1, index_reg = -1, scale = 0; // 默认base, index寄存器都不存在, 需要设置
  rtl_li(&s0, 0); // 定义在cpu.c, 定义没有提供任何信息: rtlreg_t s0, s1, t0, t1, ir; 它在这里被用于写, 用于保存地址, 但是结果还是会存到rm->addr中(内存对应的字段)

  if (m->R_M == R_ESP) { // <0> scale也只有在这种情况下才会被设置
    SIB s; // 应该还有立即数
    s.val = instr_fetch(pc, 1);
    base_reg = s.base;
    scale = s.ss;

    if (s.index != R_ESP) { index_reg = s.index; } // <1> 为什么还需要不为esp? 如果为esp, 是什么情况? 没有index寄存器的情况
    // 如果不成立, 表示index不存在, 不需要处理, 因为默认就是不存在
  }
  else {
    /* no SIB */
    base_reg = m->R_M;
  }

  if (m->mod == 0) {
    if (base_reg == R_EBP) { base_reg = -1; } // 为5的情况是立即数, 偏置也是4位, 无需设置
    else { disp_size = 0; } // 否则没有偏置
  }
  else if (m->mod == 1) { disp_size = 1; } // mod为1, 表示偏置是1 byte

  if (disp_size != 0) {
    /* has disp */
    disp = instr_fetch(pc, disp_size);
    if (disp_size == 1) { disp = (int8_t)disp; } // 类型转换

    rtl_addi(&s0, &s0, disp); // 加立即数
  }

  if (base_reg != -1) {
    rtl_add(&s0, &s0, &reg_l(base_reg)); // 加寄存器的结果
  }

  if (index_reg != -1) {
    rtl_shli(&s1, &reg_l(index_reg), scale); // s1保存索引的结果
    rtl_add(&s0, &s0, &s1); // s0+=s1
  }
  rtl_mv(&rm->addr, &s0); // 保存到addr

#ifdef DEBUG
  char disp_buf[16];
  char base_buf[8];
  char index_buf[8];

  if (disp_size != 0) {
    /* has disp */
    sprintf(disp_buf, "%s%#x", (disp < 0 ? "-" : ""), (disp < 0 ? -disp : disp));
  }
  else { disp_buf[0] = '\0'; }

  if (base_reg == -1) { base_buf[0] = '\0'; }
  else { 
    sprintf(base_buf, "%%%s", reg_name(base_reg, 4));
  }

  if (index_reg == -1) { index_buf[0] = '\0'; }
  else { 
    sprintf(index_buf, ",%%%s,%d", reg_name(index_reg, 4), 1 << scale);
  }

  if (base_reg == -1 && index_reg == -1) {
    sprintf(rm->str, "%s", disp_buf);
  }
  else {
    sprintf(rm->str, "%s(%s%s)", disp_buf, base_buf, index_buf);
  }
#endif
  rm->type = OP_TYPE_MEM;
}

/**
 * 什么时候调用? 当由opcode确定, 有modr/m byte 调用
 * 效果: 写操作数rm和reg(如果reg不为NULL的话), 不包括处理立即数, 意思是说movw  $0x1,-0x2000(%ecx,%ebx,4), 译出-0x2000(%ecx,%ebx,4)这个地址是它的事, 但是译出立即数$0x1超过了它的职权范围, 因为Modr/m byte没有这个信息
 * read_ModR_M除了调用load_addr, 仅负责处理操作数是寄存器的情况
*/
void read_ModR_M(vaddr_t *pc, Operand *rm, bool load_rm_val, Operand *reg, bool load_reg_val) {
  ModR_M m; // <0> m用于保存读取到的modr/m byte
  m.val = instr_fetch(pc, 1);
  decinfo.isa.ext_opcode = m.opcode; // <1>
  if (reg != NULL) { // <2> 与前面opcode相关, 如果这个指令的opcode指示, reg/op位表示寄存器操作数, 不为NULL, 否则为NULL, 应解释为op位
    reg->type = OP_TYPE_REG;
    reg->reg = m.reg;
    if (load_reg_val) {
      rtl_lr(&reg->val, reg->reg, reg->width);
    }

#ifdef DEBUG
    snprintf(reg->str, OP_STR_SIZE, "%%%s", reg_name(reg->reg, reg->width));
#endif
  }

  if (m.mod == 3) { // 目的是寄存器
    rm->type = OP_TYPE_REG;
    rm->reg = m.R_M; // 寄存器号
    if (load_rm_val) {
      rtl_lr(&rm->val, m.R_M, rm->width);
    }

#ifdef DEBUG
    sprintf(rm->str, "%%%s", reg_name(m.R_M, rm->width));
#endif
  }
  else {
    load_addr(pc, &m, rm); // <3> 处理mod!=3的情况, 即ModR/M是地址, 会读取SIB和立即数字段(如果有的话), 存入rm->addr中
    if (load_rm_val) {
      rtl_lm(&rm->val, &rm->addr, rm->width);
    }
  }
}
