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
static uintptr_t loader(PCB *pcb, const char *filename) {
  Elf_Ehdr elf_header;
  int fd = fs_open(filename, 0, 0); // 后两个参数没用上, 于是随便写0了
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
  for(size_t i = 0; i<ph_num; i++,ph_addr+=ph_size) {
    fs_lseek(fd, ph_addr, SEEK_SET);
    fs_read(fd, &tmp_ph, ph_size);
    // 看buffer大小是不是够了
    if(tmp_ph.p_type==PT_NULL) {
      continue;
    }
    fs_lseek(fd, tmp_ph.p_offset, SEEK_SET);
    size_t vaddr_st = tmp_ph.p_vaddr;
    size_t vaddr_end = (tmp_ph.p_vaddr+tmp_ph.p_memsz-1);
    // 这里不同于一般情况, 分配的物理页可以是连续的
    size_t mid_addr = tmp_ph.p_vaddr+tmp_ph.p_filesz-1;
    size_t cur_addr = vaddr_st;
    size_t next_addr;
    size_t len;
    void* paddr;
    while(cur_addr<mid_addr) {
      paddr = add_vmap(&pcb->as, cur_addr);
      next_addr = min(mid_addr, PGROUNDUP_GT(cur_addr));
      len = next_addr - cur_addr;
      fs_read(fd, paddr, len);
      cur_addr = next_addr;
    }
    while(cur_addr<vaddr_end) {
      paddr = add_vmap(&pcb->as, cur_addr);
      next_addr = min(mid_addr, PGROUNDUP_GT(cur_addr));
      len = next_addr - cur_addr;
      memset(paddr, 0, len);
      cur_addr = next_addr;
    }
  }

  return entry; // 返回的是入口地址
}

void naive_uload(PCB *pcb, const char *filename) {
  _protect(&pcb->as);
  uintptr_t entry = loader(pcb, filename);
  Log("Jump to entry = %x", entry);
  ((void(*)())entry) ();
}

void context_kload(PCB *pcb, void *entry, void *arg) {
  _Area stack;
  stack.start = pcb->stack;
  stack.end = stack.start + sizeof(pcb->stack);

  pcb->cp = _kcontext(stack, entry, arg);
}

void context_uload(PCB *pcb, const char *filename, char* argv[], char* envp[]) {
  _protect(&pcb->as);
  uintptr_t entry = loader(pcb, filename);

  _Area stack; // 是内核栈
  stack.start = pcb->stack;
  stack.end = stack.start + sizeof(pcb->stack);
  pcb->cp = _ucontext(&pcb->as, _heap, stack, (void *)entry, argv, envp);
}
