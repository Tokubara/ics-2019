#ifndef __X86_REG_H__
#define __X86_REG_H__

#include "common.h"
#include "nemu.h"
#include "isa/mmu.h"

#define PC_START IMAGE_START

enum { R_EAX, R_ECX, R_EDX, R_EBX, R_ESP, R_EBP, R_ESI, R_EDI };
enum { R_AX, R_CX, R_DX, R_BX, R_SP, R_BP, R_SI, R_DI };
enum { R_AL, R_CL, R_DL, R_BL, R_AH, R_CH, R_DH, R_BH };

/* TODO: Re-organize the `CPU_state' structure to match the register
 * encoding scheme in i386 instruction format. For example, if we
 * access cpu.gpr[3]._16, we will get the `bx' register; if we access
 * cpu.gpr[1]._8[1], we will get the 'ch' register. Hint: Use `union'.
 * For more details about the register encoding scheme, see i386 manual.
 */

typedef struct {
  union {
    union {
      uint32_t _32;
      uint16_t _16;
      uint8_t _8[2];
    } gpr[8];
     

    /* Do NOT change the order of the GPRs' definitions. */

    /* In NEMU, rtlreg_t is exactly uint32_t. This makes RTL instructions
     * in PA2 able to directly access these registers.
     */
    struct {
      rtlreg_t eax, ecx, edx, ebx, esp, ebp, esi, edi;
    };
  };

  vaddr_t pc;
  union {
    struct {
      rtlreg_t CF:1;
      rtlreg_t :1;
      rtlreg_t PF:1;
      rtlreg_t :1;
      rtlreg_t AF:1;
      rtlreg_t :1;
      rtlreg_t ZF:1;
      rtlreg_t SF:1;
      rtlreg_t TF:1;
      rtlreg_t IF:1;
      rtlreg_t DF:1;
      rtlreg_t OF:1;
    };

    rtlreg_t val;
  } eflags;

  vaddr_t cs, ss, ds, es, fs, gs;
  vaddr_t idt;
  vaddr_t gdt;
  vaddr_t tr;
  paddr_t tss_esp0_paddr;
  union {
    rtlreg_t cr[4];
    struct {
      CR0 cr0;
      rtlreg_t cr1;
      rtlreg_t cr2;
      CR3 cr3;
    };
  };
  bool INTR;

} CPU_state;

static inline int check_reg_index(int index) {
  assert(index >= 0 && index < 8);
  return index;
}

#define reg_l(index) (cpu.gpr[check_reg_index(index)]._32)
#define reg_w(index) (cpu.gpr[check_reg_index(index)]._16)
#define reg_b(index) (cpu.gpr[check_reg_index(index) & 0x3]._8[index >> 2])

static inline const char* reg_name(int index, int width) {
  extern const char* regsl[];
  extern const char* regsw[];
  extern const char* regsb[];
  assert(index >= 0 && index < 8);

  switch (width) {
    case 4: return regsl[index];
    case 1: return regsb[index];
    case 2: return regsw[index];
    default: assert(0);
  }
}

i32 isa_reg_str2val(const char *s, u32* val);
void isa_reg_display();

// Segment Descriptor
typedef struct SegDesc {
  uint32_t lim_15_0 : 16;  // Low bits of segment limit
  uint32_t base_15_0 : 16; // Low bits of segment base address
  uint32_t base_23_16 : 8; // Middle bits of segment base address
  uint32_t type : 4  ;     // Segment type (see STS_ constants)
  uint32_t s : 1;          // 0 = system, 1 = application
  uint32_t dpl : 2;        // Descriptor Privilege Level
  uint32_t p : 1;          // Present
  uint32_t lim_19_16 : 4;  // High bits of segment limit
  uint32_t avl : 1;        // Unused (available for software use)
  uint32_t rsv1 : 1;       // Reserved
  uint32_t db : 1;         // 0 = 16-bit segment, 1 = 32-bit segment
  uint32_t g : 1;          // Granularity: limit scaled by 4K when set
  uint32_t base_31_24 : 8; // High bits of segment base address
} SegDesc;

typedef struct TSS {
  uint32_t link;     // Unused
  uint32_t esp0;     // Stack pointers and segment selectors
  uint32_t ss0;      //   after an increase in privilege level
  char     padding[88];
} TSS;

#endif
