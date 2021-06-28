#----|------------entry------------|---irq id---|-----handler-----|
.globl __am_vecsys;    __am_vecsys: pushl $0x80; jmp __am_asm_trap
.globl __am_vectrap;  __am_vectrap: pushl $0x81; jmp __am_asm_trap
.globl __am_irq0;        __am_irq0: pushl   $32; jmp __am_asm_trap
.globl __am_vecnull;  __am_vecnull: pushl   $-1; jmp __am_asm_trap

__am_asm_trap:
  pushal

  pushl $0

  pushl %esp # 这是__am_irq_handle的参数
  call __am_irq_handle

  addl $4, %esp  # 恢复push esp

  addl $4, %esp # 恢复push 0
  popal
  addl $4, %esp # 恢复__am_vectrap传的参数

  iret
