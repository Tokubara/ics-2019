#include "proc.h" // 包含了common.h
#include "fs.h"
#include <elf.h>

#ifdef __ISA_AM_NATIVE__
# define Elf_Ehdr Elf64_Ehdr
# define Elf_Phdr Elf64_Phdr
#else
# define Elf_Ehdr Elf32_Ehdr
# define Elf_Phdr Elf32_Phdr
#endif

#define USER_STACK_END 0x48000000
#define USER_STACK_SIZE 0x8000

#define PGSIZE 4096
#define PGMASK          (PGSIZE - 1)    // Mask for bit ops
#define PGROUNDDOWN(a)  (((a)) & ~PGMASK)
#define PGROUNDUP_GT(sz)  (PGROUNDDOWN(sz)+PGSIZE)
#define min(a,b) ((a<=b)?a:b)
#define max(a,b) ((a>=b)?a:b)
static uintptr_t loader(PCB *pcb, const char *filename) {
  Elf_Ehdr elf_header;
  int fd = fs_open(filename, 0, 0); // 后两个参数没用上, 于是随便写0了
  assert(fd >= 0);
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
#ifdef HAS_VME
    size_t vaddr_end = tmp_ph.p_vaddr+tmp_ph.p_memsz; // 右开
    size_t vaddr_mid = tmp_ph.p_vaddr+tmp_ph.p_filesz; // 右开
    size_t cur_addr = vaddr_st;
    size_t next_addr;
    size_t len;
    void* paddr;
    heap_start = max(heap_start, vaddr_end);
    while(cur_addr<vaddr_mid) {
      paddr = add_vmap(&pcb->as, cur_addr);
      next_addr = min(vaddr_mid, PGROUNDUP_GT(cur_addr));
      len = next_addr - cur_addr;
      fs_read(fd, paddr, len);
      cur_addr = next_addr;
    }
    while(cur_addr<vaddr_end) {
      paddr = add_vmap(&pcb->as, cur_addr);
      next_addr = min(vaddr_end, PGROUNDUP_GT(cur_addr));
      len = next_addr - cur_addr;
      memset(paddr, 0, len);
      cur_addr = next_addr;
    }
#else
    fs_read(fd, vaddr_st, tmp_ph.p_filesz);
    if(tmp_ph.p_memsz>tmp_ph.p_filesz) {
      tmp_addr = (void*)(tmp_ph.p_vaddr+tmp_ph.p_filesz);
      memset(tmp_addr, 0, tmp_ph.p_memsz-tmp_ph.p_filesz);
    }
#endif
  }
#ifdef HAS_VME
  pcb->max_brk = heap_start;
#endif
  return entry; // 返回的是入口地址
}

void naive_uload(PCB *pcb, const char *filename) {
  _protect(&pcb->as);
  uintptr_t entry = loader(pcb, filename);
  Log("Jump to entry = %x", entry);
  pcb->cp->as = &pcb->as; 
  __am_switch(pcb->cp);
  ((void(*)())entry) ();
}

typedef uint32_t PDE;
extern PDE kpdirs[]; // 那它应该是恰好占一页
void context_kload(PCB *pcb, void *entry, void *arg, unsigned priority, unsigned pid) {
  pcb->as.ptr = kpdirs;
  pcb->status = RUNNING;
  pcb->priority = priority;
  pcb->ticks = priority;
  pcb->pid = pid;

  _Area stack;
  stack.start = pcb->stack;
  stack.end = stack.start + sizeof(pcb->stack);

  pcb->cp = _kcontext(&pcb->as, stack, entry, arg);
}

void context_uload(PCB *pcb, const char *filename, char* argv[], char* envp[], unsigned priority, unsigned pid) {
  _protect(&pcb->as);
  pcb->status = RUNNING;
  pcb->priority = priority;
  pcb->ticks = priority;
  pcb->pid = pid;
  uintptr_t entry = loader(pcb, filename);

  _Area stack; // 是内核栈
  stack.start = pcb->stack;
  stack.end = stack.start + sizeof(pcb->stack);
#ifdef HAS_VME
  void* paddr_user_stack_end = add_vmap(&pcb->as, USER_STACK_END - PGSIZE);
  add_vmap_range(&pcb->as, USER_STACK_END - USER_STACK_SIZE, USER_STACK_END - 1); // 分配用户栈空间
  _Area ustack;
  ustack.start = USER_STACK_END; // start字段用不上的, 存虚拟地址
  ustack.end = paddr_user_stack_end + PGSIZE; // 存物理地址
#else
  _Area ustack;
  ustack.end = _heap.end - pcb->pid * USER_STACK_SIZE; // 存物理地址
  ustack.start = ustack.end; // 为了与VME的情况保持一致, start与end相同
#endif
  pcb->cp = _ucontext(&pcb->as, ustack, stack, (void *)entry, argv, envp);
  Log_trace("[pid %d] kernel stack start: %x, end: %x; user stack end paddr: %x, vaddr: %x", pcb->pid, stack.start, stack.end, ustack.start, ustack.end);
}

int sys_execve(const char *filename, char *const argv[], char *const envp[]) {
  static char filename_buf[30];
  static char argv_buf[4][40];
  static char* argv_buf_ptr[4];
  static char envp_buf[4][40];
  static char* envp_buf_ptr[4];
  // 参数拷贝到内核空间, 因为之后原内存空间会被覆盖
  assert(strlen(filename)<30);
  strcpy(filename_buf, filename);
  int i = 0;
  while(argv[i] != NULL && i < 3) {
    assert(strlen(argv[i])<40);
    strcpy(argv_buf[i], argv[i]);
    argv_buf_ptr[i] = &argv_buf[i];
    ++i;
  }
  argv_buf_ptr[i] = NULL;
  i = 0;
  while(envp[i] != NULL && i < 3) {
    assert(strlen(envp[i])<40);
    strcpy(envp_buf[i], envp[i]);
    envp_buf_ptr[i] = &envp_buf[i];
    ++i;
  }
  envp_buf_ptr[i] = NULL;
  uintptr_t entry = loader(current, filename_buf);

  _Area stack; // 是内核栈
  stack.start = current->stack;
  stack.end = stack.start + sizeof(current->stack);
#ifdef HAS_VME
  size_t paddr_user_stack_end = has_map(&current->as, USER_STACK_END-1) + 1; // 之所以-1再+1, 因为END为整页数不会直接被map
  Log_debug("paddr_user_stack_end: %x", paddr_user_stack_end);
  _Area ustack;
  ustack.start = USER_STACK_END; // start字段用不上的, 存虚拟地址
  ustack.end = paddr_user_stack_end; // 存物理地址
#else
  _Area ustack;
  ustack.end = _heap.end - current->pid * USER_STACK_SIZE; // 存物理地址
  ustack.start = ustack.end; // 为了与VME的情况保持一致, start与end相同
#endif
  Log_debug("before _ucontext: %s", argv_buf[0]);
  Log_debug("before _ucontext: %s", argv_buf_ptr[0]);
  current->cp = _ucontext(&current->as, ustack, stack, (void *)entry, argv_buf_ptr, envp_buf_ptr);
  Log_trace("[pid %d] kernel stack start: %x, end: %x; user stack end paddr: %x, vaddr: %x", current->pid, stack.start, stack.end, ustack.start, ustack.end);
}
