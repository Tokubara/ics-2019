#include "cpu/exec.h"

make_EHelper(lidt) {
  rtl_addi(&s0,&id_dest->addr,2);
  unsigned idt_addr = vaddr_read(s0,4);
  rtl_mv(&cpu.idt, &idt_addr);

  print_asm_template1(lidt);
}

make_EHelper(mov_r2cr) {
  TODO();

  print_asm("movl %%%s,%%cr%d", reg_name(id_src->reg, 4), id_dest->reg);
}

make_EHelper(mov_cr2r) {
  TODO();

  print_asm("movl %%cr%d,%%%s", id_src->reg, reg_name(id_dest->reg, 4));

  difftest_skip_ref();
}

void raise_intr(uint32_t NO, vaddr_t ret_addr);
make_EHelper(int) {
  raise_intr(id_dest->val, *pc);

  print_asm("int %s", id_dest->str);

  difftest_skip_dut(1, 2);
}

make_EHelper(iret) {
  rtl_pop(&s1);
  rtl_j(s1); // 修改pc
  rtl_pop(&s1);
  rtl_mv(&cpu.cs, &s1);
  rtl_pop(&s1);
  rtl_mv(&cpu.eflags.val, &s1);

  print_asm("iret");
}

uint32_t pio_read_l(ioaddr_t);
uint32_t pio_read_w(ioaddr_t);
uint32_t pio_read_b(ioaddr_t);
void pio_write_l(ioaddr_t, uint32_t);
void pio_write_w(ioaddr_t, uint32_t);
void pio_write_b(ioaddr_t, uint32_t);

make_EHelper(in) {
  switch(id_dest->width) { // IN AX,DX, 由AX决定
    case 1:{ rtl_li(&s0, pio_read_b(id_src->val)); 
             operand_write(id_dest, &s0) ;
             break;}
    case 2:{ rtl_li(&s0, pio_read_w(id_src->val)); 
             operand_write(id_dest, &s0) ;
             break;}
    case 4:{ rtl_li(&s0, pio_read_l(id_src->val)); 
             operand_write(id_dest, &s0) ;
             break;}
    default:{panic("invalid width: %d", id_dest->width);
              break;}
  }

  print_asm_template2(in);
}

make_EHelper(out) {
  switch(id_src->width) {
    case 1:{pio_write_b(id_dest->val, id_src->val);
             break;}
    case 2:{pio_write_w(id_dest->val, id_src->val);
             break;}
    case 4:{pio_write_l(id_dest->val, id_src->val);
             break;}
    default:{panic("invalid width: %d", id_src->width);
              break;}
  }

  print_asm_template2(out);
}
