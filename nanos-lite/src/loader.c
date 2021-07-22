#include "proc.h"
#include "fs.h"
#include <elf.h>

#ifdef __ISA_AM_NATIVE__
# define Elf_Ehdr Elf64_Ehdr
# define Elf_Phdr Elf64_Phdr
#else
# define Elf_Ehdr Elf32_Ehdr
# define Elf_Phdr Elf32_Phdr
#endif

#define PGSIZE 4096
#define PGMASK          (PGSIZE - 1)    // Mask for bit ops
#define PGROUNDDOWN(a)  (((a)) & ~PGMASK)
#define PGROUNDUP_GT(sz)  (PGROUNDDOWN(sz)+PGSIZE)
#define min(a,b) ((a<=b)?a:b)
#define max(a,b) ((a>=b)?a:b)
static uintptr_t loader(PCB *pcb, const char *filename) {
  Elf_Ehdr elf_header;
  int fd = fs_open(filename, 0, 0); // 后两个参数没用上, 于是随便写0了
  if (fd<0) {
    Log_error("%s not exists", filename);
    _halt(1);
  }
  fs_read(fd, &elf_header, sizeof(Elf_Ehdr));
  uintptr_t entry = elf_header.e_entry;
  size_t ph_num = elf_header.e_phnum;
  Elf_Phdr tmp_ph; // 如果开辟一个数组, 就可以读整个
  if(sizeof(Elf_Phdr)!=elf_header.e_phentsize) {
    printf("Error: sizeof(Elf_Phdr)!=elf_header.e_phentsize\n,left=%u,right=%u\n",sizeof(Elf_Phdr),elf_header.e_phentsize);
    _halt(1);
  }
  uintptr_t ph_addr = elf_header.e_phoff;
  size_t ph_size = sizeof(Elf_Phdr);
  void* tmp_addr = NULL;
  size_t heap_start = 0;
  for(size_t i = 0; i<ph_num; i++,ph_addr+=ph_size) {
    fs_lseek(fd, ph_addr, SEEK_SET);
    fs_read(fd, &tmp_ph, ph_size);
    if(tmp_ph.p_type==PT_NULL || tmp_ph.p_vaddr==0) {
      continue;
    }
    fs_lseek(fd, tmp_ph.p_offset, SEEK_SET);
    
    size_t vaddr_st = tmp_ph.p_vaddr;
    size_t vaddr_end = tmp_ph.p_vaddr+tmp_ph.p_memsz; // 右开
    size_t vaddr_mid = tmp_ph.p_vaddr+tmp_ph.p_filesz; // 右开
    size_t cur_addr = vaddr_st;
    size_t next_addr;
    size_t len;
    void* paddr;
    heap_start = max(heap_start, vaddr_end);
    // Log_debug("vaddr_st:%x,vaddr_mid:%x, vaddr_end:%x", vaddr_st, vaddr_mid, vaddr_end);
    // Log_debug("offset:%x", tmp_ph.p_offset);
    while(cur_addr<vaddr_mid) {
      paddr = add_vmap(&pcb->as, cur_addr);
      next_addr = min(vaddr_mid, PGROUNDUP_GT(cur_addr));
      len = next_addr - cur_addr;
      fs_read(fd, paddr, len);
      // Log_debug("paddr:%x,len:%d", paddr, len);
      cur_addr = next_addr;
    }
    while(cur_addr<vaddr_end) {
      paddr = add_vmap(&pcb->as, cur_addr);
      next_addr = min(vaddr_end, PGROUNDUP_GT(cur_addr));
      len = next_addr - cur_addr;
      memset(paddr, 0, len);
      cur_addr = next_addr;
    }
  }
  pcb->max_brk = heap_start;
  Log_debug("max_brk:%x", heap_start);
  return entry; // 返回的是入口地址
}

void naive_uload(PCB *pcb, const char *filename) {
  _protect(&pcb->as);
  uintptr_t entry = loader(pcb, filename);
  Log("Jump to entry = %x", entry);
  pcb->cp->as = &pcb->as; 
  // Log_debug("pcb->cp->as(%x) = &pcb->as(%x);", pcb->cp->as, &pcb->as);
  // Log_debug("pcb->cp->as->ptr(%x) = pcb->as.ptr(%x); (&pcb->as)->ptr(%x)", pcb->cp->as->ptr, pcb->as.ptr, (&pcb->as)->ptr);
  __am_switch(pcb->cp);
  ((void(*)())entry) ();
}

typedef uint32_t PDE;
extern PDE kpdirs[]; // 那它应该是恰好占一页
void context_kload(PCB *pcb, void *entry, void *arg) {
  pcb->as.ptr = kpdirs;
  _Area stack;
  stack.start = pcb->stack;
  stack.end = stack.start + sizeof(pcb->stack);

  pcb->cp = _kcontext(&pcb->as, stack, entry, arg);
}

void context_uload(PCB *pcb, const char *filename, char* argv[], char* envp[]) {
  Log_debug("enter");
  _protect(&pcb->as);
  uintptr_t entry = loader(pcb, filename);

  _Area stack; // 是内核栈
  stack.start = pcb->stack;
  stack.end = stack.start + sizeof(pcb->stack);
  pcb->cp = _ucontext(&pcb->as, _heap, stack, (void *)entry, argv, envp);
}
