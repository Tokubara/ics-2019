#include "nemu.h"
#include "isa/mmu.h"

bool page_translate(vaddr_t vaddr, paddr_t* paddr) {
  addr_t addr;
  addr.val = vaddr;
  PDE* pde_arr = (PDE*)((uintptr_t)(cpu.cr3.page_directory_base << 12));
  PDE pde = pde_arr[addr.hi];
  if (pde.present!=1) {
    return false;
  }
  PTE* pte_arr = (PTE*)((uintptr_t)(pde.page_frame << 12));
  PTE pte = pte_arr[addr.mid];
  if (pte.present!=1) {
    return false;
  }
  *paddr = pte.page_frame << 12 | addr.lo;
  return true;
}

uint32_t isa_vaddr_read(vaddr_t addr, int len) {
  paddr_t paddr;
  if (cpu.cr0.paging==1) {
    // 判断是否跨页
    assert((addr&0xfffff000)==((addr+len-1)&0xfffff000)); // 不处理跨页的情况
    bool ret = page_translate(addr, &paddr);
    assert(ret==true);
  } else {
    paddr = addr;
  }
  return paddr_read(paddr, len);
}

void isa_vaddr_write(vaddr_t addr, uint32_t data, int len) {
  paddr_t paddr;
  if (cpu.cr0.paging==1) {
    // 判断是否跨页
    assert((addr&0xfffff000)==((addr+len-1)&0xfffff000)); // 不处理跨页的情况
    bool ret = page_translate(addr, &paddr);
    assert(ret==true);
  } else {
    paddr = addr;
  }
  paddr_write(addr, data, len);
}
