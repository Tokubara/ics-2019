#include "cpu/exec.h"

make_EHelper(add) {
  rtl_mv(&s2, &id_dest->val); // s2 存dest初始值
  rtl_add(&s1, &id_dest->val, &id_src->val);

  operand_write(id_dest, &s1);

  if (id_dest->width != 4) {
    rtl_andi(&s1, &s1, 0xffffffffu >> ((4 - id_dest->width) * 8));
  }

  rtl_update_ZFSF(&s1, id_dest->width); // 是const

  // update CF
  rtl_is_add_carry(&s0, &s1, &id_src->val, id_dest->width);
  rtl_set_CF(&s0);

  // update OF
  rtl_is_add_overflow(&s0, &id_dest->val, &s2, &id_src->val, id_dest->width);
  rtl_set_OF(&s0);

  print_asm_template2(add);
}

make_EHelper(sub) {
  rtl_mv(&s2, &id_dest->val); // s2 存dest初始值
  rtl_sub(&s1, &id_dest->val, &id_src->val);

  operand_write(id_dest, &s1);

  if (id_dest->width != 4) {
    rtl_andi(&s1, &s1, 0xffffffffu >> ((4 - id_dest->width) * 8));
  }

  rtl_update_ZFSF(&s1, id_dest->width); // 是const

  // update CF
  rtl_is_sub_carry(&s0, &s1, &s2, id_dest->width);
  rtl_set_CF(&s0);

  // update OF
  rtl_is_sub_overflow(&s0, &s1, &s2, &id_src->val, id_dest->width);
  rtl_set_OF(&s0);

  print_asm_template2(sub);
}

make_EHelper(cmp) {
  rtl_sext(&s2, &id_src->val, id_src->width); // s2存src符号扩展的结果, 因为id_dest不会修改, 不需要保存
  rtl_sub(&s1, &id_dest->val, &s2); // 是修改它么?

  if (id_dest->width != 4) {
    rtl_andi(&s1, &s1, 0xffffffffu >> ((4 - id_dest->width) * 8));
  }

  rtl_update_ZFSF(&s1, id_dest->width); // 是const

  // update CF
  rtl_is_sub_carry(&s0, &s1, &id_dest->val, id_dest->width);
  rtl_set_CF(&s0);

  // update OF
  rtl_is_sub_overflow(&s0, &s1, &id_dest->val, &s2, id_dest->width);
  rtl_set_OF(&s0);

  print_asm_template2(cmp);
}

make_EHelper(inc) {
  rtl_mv(&s2, &id_dest->val); // s2 存dest初始值
  rtl_addi(&s1, &id_dest->val, 1);

  operand_write(id_dest, &s1);

  if (id_dest->width != 4) {
    rtl_andi(&s1, &s1, 0xffffffffu >> ((4 - id_dest->width) * 8));
  }

  rtl_update_ZFSF(&s1, id_dest->width); // 是const

  // s0存为1
  rtl_li(&s0, 1);
  // update OF
  rtl_is_add_overflow(&s0, &s1, &s2, &s0, id_dest->width);
  rtl_set_OF(&s0);

  print_asm_template1(inc);
}

make_EHelper(dec) { // 拷贝自sub
  rtl_mv(&s2, &id_dest->val); // s2 存dest初始值
  rtl_subi(&s1, &id_dest->val, 1); // s1 存结果

  operand_write(id_dest, &s1);

  if (id_dest->width != 4) {
    rtl_andi(&s1, &s1, 0xffffffffu >> ((4 - id_dest->width) * 8));
  }

  rtl_update_ZFSF(&s1, id_dest->width);

  // update OF
  rtl_li(&s0, 1);
  rtl_is_sub_overflow(&s0, &s1, &s2, &s0, id_dest->width);
  rtl_set_OF(&s0);

  print_asm_template1(dec);
}

make_EHelper(neg) {
  TODO();

  print_asm_template1(neg);
}

make_EHelper(adc) {
  rtl_mv(&s2, &id_dest->val); // s2 存dest初始值
  // s0 = dest + src
  rtl_add(&s0, &id_dest->val, &id_src->val);
  // s1 = s0 + CF
  rtl_get_CF(&s1);
  rtl_add(&s1, &s0, &s1);

  operand_write(id_dest, &s1); // operand_write已经处理了操作数宽度

  if (id_dest->width != 4) { // 这一行和上一行operand_write能不能交换一下, 我感觉也行
    rtl_andi(&s1, &s1, 0xffffffffu >> ((4 - id_dest->width) * 8));
  }

  rtl_update_ZFSF(&s1, id_dest->width); // 更新ZF和SF只需要s1

  // update CF
  rtl_is_add_carry(&s1, &s1, &s0, id_dest->width); // 此时, s0还是dest+src, s1是结果
  rtl_is_add_carry(&s0, &s0, &id_src->val, id_dest->width); // init这里是id_dest, 但我觉得是bug, 改为了src
  rtl_or(&s0, &s0, &s1);
  rtl_set_CF(&s0);

  // update OF
  rtl_is_add_overflow(&s0, &id_dest->val, &s2, &id_src->val, id_dest->width); // 这我改了实现, nemu之前不是这样实现的, 我觉得之前的实现是错的
  rtl_set_OF(&s0);

  print_asm_template2(adc);
}

/**
 * 
 * 实现上来说, 与adc的差别可以说是用sub替换掉add
*/
make_EHelper(sbb) {
  rtl_mv(&s2, &id_dest->val); // s2 存dest初始值
  // s0 = dest - src
  rtl_sub(&s0, &id_dest->val, &id_src->val);
  // s1 = s0 - CF
  rtl_get_CF(&s1);
  rtl_sub(&s1, &s0, &s1);

  operand_write(id_dest, &s1);

  if (id_dest->width != 4) {
    rtl_andi(&s1, &s1, 0xffffffffu >> ((4 - id_dest->width) * 8));
  }

  rtl_update_ZFSF(&s1, id_dest->width);

  // update CF
  rtl_is_sub_carry(&s1, &s1, &s0, id_dest->width);
  rtl_is_sub_carry(&s0, &s0, &s2, id_dest->width);
  rtl_or(&s0, &s0, &s1);
  rtl_set_CF(&s0);

  // update OF
  rtl_is_sub_overflow(&s0, &s1, &s2, &id_src->val, id_dest->width);
  rtl_set_OF(&s0);

  print_asm_template2(sbb);
}

// 只用到了id_dest
make_EHelper(mul) {
  rtl_lr(&s0, R_EAX, id_dest->width);
  rtl_mul_lo(&s1, &id_dest->val, &s0);

  switch (id_dest->width) { // 为什么这里需要一个专门的switch语句, 这是因为乘法指令对于不同的操作数宽度, 很不一样
    case 1:
      rtl_sr(R_AX, &s1, 2);
      break;
    case 2:
      rtl_sr(R_AX, &s1, 2);
      rtl_shri(&s1, &s1, 16);
      rtl_sr(R_DX, &s1, 2);
      break;
    case 4:
      rtl_mul_hi(&s0, &id_dest->val, &s0);
      rtl_sr(R_EDX, &s0, 4);
      rtl_sr(R_EAX, &s1, 4);
      break;
    default: assert(0);
  }

  // 注意到没有设置OF和CF
  print_asm_template1(mul);
}

// imul with one operand
// 实现上与mul没有区别, 除了把mul替换为imul之外
make_EHelper(imul1) {
  rtl_lr(&s0, R_EAX, id_dest->width);
  rtl_imul_lo(&s1, &id_dest->val, &s0);

  switch (id_dest->width) {
    case 1:
      rtl_sr(R_AX, &s1, 2);
      break;
    case 2:
      rtl_sr(R_AX, &s1, 2);
      rtl_shri(&s1, &s1, 16);
      rtl_sr(R_DX, &s1, 2);
      break;
    case 4:
      rtl_imul_hi(&s0, &id_dest->val, &s0);
      rtl_sr(R_EDX, &s0, 4);
      rtl_sr(R_EAX, &s1, 4);
      break;
    default: assert(0);
  }

  print_asm_template1(imul);
}

// imul with two operands
make_EHelper(imul2) {
  rtl_sext(&s0, &id_src->val, id_src->width);
  rtl_sext(&s1, &id_dest->val, id_dest->width);

  rtl_imul_lo(&s0, &s1, &s0);
  operand_write(id_dest, &s0);

  print_asm_template2(imul);
}

// imul with three operands
make_EHelper(imul3) {
  rtl_sext(&s0, &id_src->val, id_src->width);
  rtl_sext(&s1, &id_src2->val, id_src->width);

  rtl_imul_lo(&s0, &s1, &s0);
  operand_write(id_dest, &s0);

  print_asm_template3(imul);
}

make_EHelper(div) {
  switch (id_dest->width) {
    case 1:
      rtl_lr(&s0, R_AX, 2);
      rtl_div_q(&s1, &s0, &id_dest->val);
      rtl_sr(R_AL, &s1, 1);
      rtl_div_r(&s1, &s0, &id_dest->val);
      rtl_sr(R_AH, &s1, 1);
      break;
    case 2:
      rtl_lr(&s0, R_AX, 2);
      rtl_lr(&s1, R_DX, 2);
      rtl_shli(&s1, &s1, 16);
      rtl_or(&s0, &s0, &s1);
      rtl_div_q(&s1, &s0, &id_dest->val);
      rtl_sr(R_AX, &s1, 2);
      rtl_div_r(&s1, &s0, &id_dest->val);
      rtl_sr(R_DX, &s1, 2);
      break;
    case 4:
      rtl_lr(&s0, R_EAX, 4);
      rtl_lr(&s1, R_EDX, 4);
      rtl_div64_q(&cpu.eax, &s1, &s0, &id_dest->val);
      rtl_div64_r(&cpu.edx, &s1, &s0, &id_dest->val);
      break;
    default: assert(0);
  }

  print_asm_template1(div);
}

make_EHelper(idiv) {
  switch (id_dest->width) {
    case 1:
      rtl_lr(&s0, R_AX, 2);
      rtl_idiv_q(&s1, &s0, &id_dest->val);
      rtl_sr(R_AL, &s1, 1);
      rtl_idiv_r(&s1, &s0, &id_dest->val);
      rtl_sr(R_AH, &s1, 1);
      break;
    case 2:
      rtl_lr(&s0, R_AX, 2);
      rtl_lr(&s1, R_DX, 2);
      rtl_shli(&s1, &s1, 16);
      rtl_or(&s0, &s0, &s1);
      rtl_idiv_q(&s1, &s0, &id_dest->val);
      rtl_sr(R_AX, &s1, 2);
      rtl_idiv_r(&s1, &s0, &id_dest->val);
      rtl_sr(R_DX, &s1, 2);
      break;
    case 4:
      rtl_lr(&s0, R_EAX, 4);
      rtl_lr(&s1, R_EDX, 4);
      rtl_idiv64_q(&cpu.eax, &s1, &s0, &id_dest->val);
      rtl_idiv64_r(&cpu.edx, &s1, &s0, &id_dest->val);
      break;
    default: assert(0);
  }

  print_asm_template1(idiv);
}
