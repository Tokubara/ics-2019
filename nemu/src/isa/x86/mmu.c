#include "nemu.h"
#include "isa/mmu.h"

bool page_translate(vaddr_t vaddr, paddr_t* paddr) {
  addr_t addr;
  addr.val = vaddr;
  PDE pde = {.val=paddr_read((cpu.cr3.page_directory_base << 12) + (addr.hi << 2), 4)};
  if (pde.present!=1) {
    return false;
  }
  PTE pte = {.val=paddr_read((pde.page_frame << 12) + (addr.mid << 2), 4)};
  if (pte.present!=1) {
    return false;
  }
  *paddr = (pte.page_frame << 12) | addr.lo;
  if(vaddr>0x40000000) {
    Log_debug("vaddr:%x, paddr:%x", vaddr, *paddr);
  }
  return true;
}


uint32_t isa_vaddr_read(vaddr_t addr, int len) {
  paddr_t paddr;
  if (cpu.cr0.paging==1) {
    bool ret;
    // 判断是否跨页
    if ((addr&0xfffff000)!=((addr+len-1)&0xfffff000)) {
      uint32_t addr_round_up = PGROUNDUP(addr);
      ret = page_translate(addr, &paddr);
      assert(ret==true);
      paddr_t paddr_hi;
      ret = page_translate(addr_round_up, &paddr_hi);
      assert(ret==true);
      int len_lo = addr_round_up-addr;
      int len_hi = 4 - len_lo;
      return paddr_read(paddr, len_lo) | (paddr_read(paddr_hi, len_hi) << len_lo);
    } else {
      ret = page_translate(addr, &paddr);
      assert(ret==true);
    }
  } else {
    paddr = addr;
  }
  return paddr_read(paddr, len);
}

void isa_vaddr_write(vaddr_t addr, uint32_t data, int len) {
  paddr_t paddr;
  if (cpu.cr0.paging==1) {
    bool ret;
    if ((addr&0xfffff000)!=((addr+len-1)&0xfffff000)) {
      uint32_t addr_round_up = PGROUNDUP(addr);
      ret = page_translate(addr, &paddr);
      assert(ret==true);
      paddr_t paddr_hi;
      ret = page_translate(addr_round_up, &paddr_hi);
      assert(ret==true);
      int len_lo = addr_round_up - addr;
      int len_hi = 4 - len_lo;
      paddr_write(paddr, data & (~0u >> (len_hi << 3)), len_lo);
      paddr_write(paddr_hi, data >> (len_lo << 3), len_hi);
    } else {
      ret = page_translate(addr, &paddr);
      assert(ret==true);
    }
  } else {
    paddr = addr;
  }
  paddr_write(paddr, data, len);
}

// static inline void* get_host_addr_by_addr(vaddr_t st_addr, size_t len) {
//   void* host_addr = get_mmio_host_addr_by_addr(st_addr, st_addr+len-1);
//   if (host_addr == NULL) {

//   }
//   if (map_inside(&pmem_map, st_addr) && map_inside(&pmem_map, st_addr+len-1)) {
//     host_addr = pmem + (st_addr - pmem_map.low);
//   } else {
//   }
//   return host_addr;
// }

#define min(a,b) ((a<=b)?a:b)
// movsb, 失败返回-1, 这样movsb的执行函数调用rtl_exit
size_t vmem_cpy(vaddr_t dest_vaddr, vaddr_t src_vaddr, size_t tar_len) {
  size_t cur_len = 0;
  paddr_t dest_paddr, src_paddr;
  bool ret;
  size_t len1, len2, len;
  while(cur_len < tar_len) {
    ret = page_translate(dest_vaddr, &dest_paddr);
    assert(ret);
    ret = page_translate(src_vaddr, &src_paddr);
    assert(ret);
    len1 = PGROUNDUP_GT(dest_vaddr) - dest_vaddr;
    len2 = PGROUNDUP_GT(src_vaddr) - src_vaddr;
    len = min(len1, len2);
    len = min(len, tar_len);
    assert(len>0);
    pmem_cpy(dest_paddr, src_paddr, len);
    dest_vaddr += len;
    src_vaddr += len;
    cur_len += len;
  }
  return tar_len;
}
