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

make_EHelper(cld) {
  // 不是不可以用rtl指令, 不过这样反而麻烦了, 还需要做位运算
  cpu.eflags.DF = 0;
  print_asm_template1(cld);
}

void isa_exec(vaddr_t *pc);
make_EHelper(rep) {
  print_asm_template1(rep); // 得放第一行, 否则与movsb顺序就是反的
  decinfo.is_rep = true;
  isa_exec(pc);
  decinfo.is_rep = false;
}

make_EHelper(movs) {
  if(decinfo.is_rep == true) {
    // s1源地址
    // s2目的地址
    Assert(cpu.eflags.DF==0, "you need to take DF=1 in consideration");
    rtl_lr(&s1, R_ESI, 4);
    rtl_lr(&s2, R_EDI, 4);
    // 必须需要4个寄存器, 于是增加了s3
    rtl_lr(&s3, R_ECX, 4); // s3表示len
    rtl_mul_loi(&s3, &s3, id_dest->width); // 这里有32位存不下的风险

    rtl_memcpy(&s0, &s2, &s1, s3); 
    if(s0 != s3) {
      // void interpret_rtl_exit(int state, vaddr_t halt_pc, uint32_t halt_ret)
      Log("error: bad addr");
      rtl_exit(NEMU_END, cpu.pc, -1);
    } else {
      // 修改edi, esi, 它们已经增加了, 此时s1和s2还没有变
      rtl_add(&s1, &s1, &s3);
      rtl_sr(R_ESI, &s1, 4);
      rtl_add(&s2, &s2, &s3);
      rtl_sr(R_EDI, &s2, 4);

      // 第2个参数还是指针
      rtl_li(&s1, 0);
      rtl_sr(R_ECX, &s1, 4);
    }

  } else {
    // mov
    rtl_lr(&s1, R_ESI, 4); // esi表示的是源地址, s1存
    rtl_lm(&s0, &s1, id_dest->width); // 确认了interpret_rtl_lm的实现, 不会导致错误
    rtl_lr(&s2, R_EDI, 4);
    rtl_sm(&s2, &s0, id_dest->width);
    // 修改edi和esi
    s0 = cpu.eflags.DF==0?id_dest->width:-id_dest->width; 
    rtl_add(&s1, &s1, &s0);
    rtl_sr(R_ESI, &s1, 4);
    rtl_add(&s2, &s2, &s0);
    rtl_sr(R_EDI, &s2, 4);
  }

  switch(id_dest->width) {
    case 1:{print_asm_template1(movsb);break;}
    case 2:{print_asm_template1(movsw);break;}
    case 4:{print_asm_template1(movsl);break;}
    default:{panic("impossible");}
  }
}

make_EHelper(mov_E2cr) {
  assert(id_dest->reg!=1);
  rtl_mv(&cpu.cr[id_dest->reg], &id_src->val);
  if(id_dest->reg==0&&cpu.cr0.paging==1) {
    Log("paging enable");
  } else if(id_dest->reg==3) {
    Log("cr3:%u", id_src->val);
  }
  print_asm("movl %s,%%cr%d", id_src->str, id_dest->reg);
}

make_EHelper(mov_cr2E) {
  assert(id_src->reg!=1);
  operand_write(id_dest, &cpu.cr[id_src->reg]);
  print_asm("movl %%cr%d,%s", id_src->reg, id_dest->str);
}
