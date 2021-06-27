.section entry, "ax"
.globl _start
# .globl _start_minus
.type _start, @function

_start:
# _start_minus equ _start - 0x100000
  mov $0, %ebp
  mov $_stack_pointer, %esp
  call _trm_init                 # never return
