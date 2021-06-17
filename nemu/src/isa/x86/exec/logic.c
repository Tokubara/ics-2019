#include "cpu/exec.h"
#include "cc.h"

make_EHelper(test) { // 拷贝自and, 只去除了operand_write这句
	rtl_and(&s1, &id_dest->val, &id_src->val);

  if (id_dest->width != 4) {
    rtl_andi(&s1, &s1, 0xffffffffu >> ((4 - id_dest->width) * 8));
  }

  rtl_update_ZFSF(&s1, id_dest->width); // 是const

  // update CF, OF
  rtl_li(&s0, 0);
  rtl_set_CF(&s0);
  rtl_set_OF(&s0);

	rtl_update_PF(&s1);

  print_asm_template2(test);
}

make_EHelper(and) { // 拷贝自xor, 只改了第一行和最后一行
	rtl_and(&s1, &id_dest->val, &id_src->val);

  operand_write(id_dest, &s1);

  if (id_dest->width != 4) {
    rtl_andi(&s1, &s1, 0xffffffffu >> ((4 - id_dest->width) * 8));
  }

  rtl_update_ZFSF(&s1, id_dest->width); // 是const

  // update CF, OF
  rtl_li(&s0, 0);
  rtl_set_CF(&s0);
  rtl_set_OF(&s0);

	rtl_update_PF(&s1);

  print_asm_template2(and);
}

make_EHelper(xor) {
	rtl_xor(&s1, &id_dest->val, &id_src->val);

  operand_write(id_dest, &s1);

  if (id_dest->width != 4) {
    rtl_andi(&s1, &s1, 0xffffffffu >> ((4 - id_dest->width) * 8));
  }

  rtl_update_ZFSF(&s1, id_dest->width); // 是const

  // update CF, OF
  rtl_li(&s0, 0);
  rtl_set_CF(&s0);
  rtl_set_OF(&s0);

	rtl_update_PF(&s1);

  print_asm_template2(xor);
}

make_EHelper(or) { // 拷贝自xor, 只改了第一行
	rtl_or(&s1, &id_dest->val, &id_src->val);

  operand_write(id_dest, &s1);

  if (id_dest->width != 4) {
    rtl_andi(&s1, &s1, 0xffffffffu >> ((4 - id_dest->width) * 8));
  }

  rtl_update_ZFSF(&s1, id_dest->width); // 是const

  // update CF, OF
  rtl_li(&s0, 0);
  rtl_set_CF(&s0);
  rtl_set_OF(&s0);

	rtl_update_PF(&s1);

  print_asm_template2(or);
}

make_EHelper(sar) {
  rtl_andi(&s0,&id_src->val,0b11111u);
  rtl_sar(&s1,&id_dest->val,&s0); // s1存着结果
  operand_write(id_dest, &s1);

  if (id_dest->width != 4) {
    rtl_andi(&s1, &s1, 0xffffffffu >> ((4 - id_dest->width) * 8));
  }

  rtl_update_ZFSF(&s1, id_dest->width); // 是const
  rtl_update_PF(&s1);
  // unnecessary to update CF and OF in NEMU
  // 由于注释说不用写, 我就真的没写

  print_asm_template2(sar);
}

make_EHelper(shl) { // copy自sar
  rtl_andi(&s0,&id_src->val,0b11111u);
  rtl_shl(&s1,&id_dest->val,&s0); // s1存着结果
  operand_write(id_dest, &s1);

  if (id_dest->width != 4) {
    rtl_andi(&s1, &s1, 0xffffffffu >> ((4 - id_dest->width) * 8));
  }

  rtl_update_ZFSF(&s1, id_dest->width); // 是const
  rtl_update_PF(&s1);
  // unnecessary to update CF and OF in NEMU

  print_asm_template2(shl);
}

make_EHelper(shr) {
  TODO();
  // unnecessary to update CF and OF in NEMU

  print_asm_template2(shr);
}

make_EHelper(setcc) {
  uint32_t cc = decinfo.opcode & 0xf;

  rtl_setcc(&s0, cc);
  operand_write(id_dest, &s0);

  print_asm("set%s %s", get_cc_name(cc), id_dest->str);
}

make_EHelper(not) {
  // 不影响 eflags
  rtl_mv(&s0, &id_dest->val);
  rtl_li(&s0, ~s0);
  operand_write(id_dest, &s0);

  print_asm_template1(not);
}
