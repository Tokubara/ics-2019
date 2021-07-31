#include "cpu/exec.h"
#include "nemu.h"

// 只存了地址信息, 没有存界限信息, 按理说也该存段界限的
make_EHelper(lidt) {
  rtl_addi(&s0, &id_dest->addr, 2); // 注意这里用的是addr字段, 而不是val字段 // +2是因为低2 bytes是段界限, 需要跳过段界限
  unsigned idt_addr = vaddr_read(s0, 4);
  rtl_mv(&cpu.idt, &idt_addr); // 其实这里既是虚拟地址也是物理地址, 因为这是在内核中

  print_asm_template1(lidt);
}

// copy from lidt, 因此也没有存段界限
make_EHelper(lgdt) {
  rtl_addi(&s0, &id_dest->addr, 2);
  unsigned gdt_addr = vaddr_read(s0, 4);
  rtl_mv(&cpu.gdt, &gdt_addr);

  print_asm_template1(lgdt);
}

make_EHelper(ltr) {
  rtl_seg_desc_addr(&cpu.tr, id_dest->val);
  // Log_debug("cpu.tr: %x", cpu.tr);

  bool ret = page_translate(cpu.tr+offset(TSS, esp0),  &cpu.tss_esp0_paddr);
  Assert_vaddr(cpu.tr+offset(TSS, esp0));
  // Log_debug("esp0 paddr: %x", cpu.tss_esp0_paddr);

  print_asm_template1(ltr);
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
  rtl_pop(&s1); // s1存着esp
  rtl_pop(&s0); // 用不上, 随后被覆盖
  rtl_lm_ph(&s0, &cpu.tss_esp0_paddr, 4);
  if (s1 == 0) {
    Log_debug("esp not change, esp = %x, cr3=%x", cpu.esp, cpu.cr3.val);
    // Assert(s0 == 0, "esp0 in tss: %x", s0);
  } else {
    assert(s0 != 0);
    Log_debug("old esp=%x, new esp=%x", cpu.esp, s1);
    rtl_mv(&cpu.esp, &s1);
  }

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
