#ifndef __ARCH_H__
#define __ARCH_H__

struct _Context { // 后压入的在先
  struct _AddressSpace *as;
  uintptr_t edi, esi, ebp, _esp, ebx, edx, ecx, eax;
  int irq;
  uintptr_t eip, cs, eflags, esp, ss;
};

#define GPR1 eax
#define GPR2 ebx
#define GPR3 ecx
#define GPR4 edx
#define GPRx eax

#endif
