#include "cpu/exec.h"

make_EHelper(mov) {
  operand_write(id_dest, &id_src->val);
  print_asm_template2(mov);
}

make_EHelper(push) {
  rtl_push(&id_dest->val); // 译码的时候是译在dest中的
  print_asm_template1(push);
}

make_EHelper(pop) {
  rtl_pop(&s0);
  operand_write(id_dest, &s0);
  
  print_asm_template1(pop);
}

make_EHelper(pusha) {
  rtl_lr(&s0, R_ESP, id_dest->width); // s0作为temp, 保存esp的初始值
  rtl_lr(&s1, R_EAX, id_dest->width); // s1是临时的
  rtl_push(&s1);
  rtl_lr(&s1, R_ECX, id_dest->width);
  rtl_push(&s1);
  rtl_lr(&s1, R_EDX, id_dest->width);
  rtl_push(&s1);
  rtl_lr(&s1, R_EBX, id_dest->width);
  rtl_push(&s1);

  rtl_push(&s0); // 存temp

  rtl_lr(&s1, R_EBP, id_dest->width);
  rtl_push(&s1);
  rtl_lr(&s1, R_ESI, id_dest->width);
  rtl_push(&s1);
  rtl_lr(&s1, R_EDI, id_dest->width);
  rtl_push(&s1);

  print_asm("pusha");
}

make_EHelper(popa) {
  rtl_pop(&s1);
  rtl_sr(R_EDI, &s1, id_dest->width);
  rtl_pop(&s1);
  rtl_sr(R_ESI, &s1, id_dest->width);
  rtl_pop(&s1);
  rtl_sr(R_EBP, &s1, id_dest->width);

  rtl_pop(&s1); // 跳过sp

  rtl_pop(&s1);
  rtl_sr(R_EBX, &s1, id_dest->width);
  rtl_pop(&s1);
  rtl_sr(R_EDX, &s1, id_dest->width);
  rtl_pop(&s1);
  rtl_sr(R_ECX, &s1, id_dest->width);
  rtl_pop(&s1);
  rtl_sr(R_EAX, &s1, id_dest->width);

  print_asm("popa");
}

make_EHelper(leave) {
  // 注意需要考虑decinfo.width
  // 首先是恢复esp, 然后pop
  if(decinfo.width==2) {
    rtl_lr(&s0,R_BP,2); // 载入ebp的值
    rtl_sr(R_SP,&s0,2);
    rtl_pop(&s0);
    rtl_sr(R_BP,&s0,2);
  } else {
    rtl_lr(&s0,R_EBP,4);
    rtl_sr(R_ESP,&s0,4);
    rtl_pop(&s0);
    rtl_sr(R_EBP,&s0,4);
  }
  print_asm("leave");
}

make_EHelper(cltd) {
  if (decinfo.isa.is_operand_size_16) {
    rtl_lr(&s0, R_AX, 2);
    rtl_sext(&s0, &s0, 2);
    rtl_shri(&s0, &s0, 16);
    rtl_sr(R_DX, &s0, 2);
  }
  else {
    rtl_lr(&s0, R_EAX, 4);
    rtl_shri(&s0, &s0, 31);
    if(s0>0) {
      rtl_li(&s1, 0xffffffff);
    } else {
      rtl_li(&s1, 0);
    }
    rtl_sr(R_EDX, &s1, 4);
  }

  print_asm(decinfo.isa.is_operand_size_16 ? "cwtl" : "cltd");
}

make_EHelper(cwtl) {
  if (decinfo.isa.is_operand_size_16) {
    TODO();
  }
  else {
    TODO();
  }

  print_asm(decinfo.isa.is_operand_size_16 ? "cbtw" : "cwtl");
}

make_EHelper(movsx) {
  id_dest->width = decinfo.isa.is_operand_size_16 ? 2 : 4;
  rtl_sext(&s0, &id_src->val, id_src->width);
  operand_write(id_dest, &s0);
  print_asm_template2(movsx);
}

make_EHelper(movzx) {
  id_dest->width = decinfo.isa.is_operand_size_16 ? 2 : 4;
  operand_write(id_dest, &id_src->val);
  print_asm_template2(movzx);
}

make_EHelper(lea) {
  operand_write(id_dest, &id_src->addr);
  print_asm_template2(lea);
}
