#include <am.h>
#include <x86.h>
#include <nemu.h>
#include <debug.h>

#define PG_ALIGN __attribute((aligned(PGSIZE)))

PDE kpdirs[NR_PDE] PG_ALIGN = {}; // 那它应该是恰好占一页
static PTE kptabs[(PMEM_SIZE + MMIO_SIZE) / PGSIZE] PG_ALIGN = {};
static void* (*pgalloc_usr)(size_t) = NULL;
static void (*pgfree_usr)(void*) = NULL;
static int vme_enable = 0;

static _Area segments[] = {      // Kernel memory mappings
  {.start = (void*)0,          .end = (void*)PMEM_SIZE},
  {.start = (void*)MMIO_BASE,  .end = (void*)(MMIO_BASE + MMIO_SIZE)}
};

#define NR_KSEG_MAP (sizeof(segments) / sizeof(segments[0])) // 段数

int _vme_init(void* (*pgalloc_f)(size_t), void (*pgfree_f)(void*)) {
  pgalloc_usr = pgalloc_f;
  pgfree_usr = pgfree_f;

  int i;

  // make all PDEs invalid
  for (i = 0; i < NR_PDE; i ++) {
    kpdirs[i] = 0;
  }

  PTE *ptab = kptabs;
  for (i = 0; i < NR_KSEG_MAP; i ++) {
    uint32_t pdir_idx = (uintptr_t)segments[i].start / (PGSIZE * NR_PTE); // 得到的是地址的最高10位
    uint32_t pdir_idx_end = (uintptr_t)segments[i].end / (PGSIZE * NR_PTE); // 按理说这个也是应该要覆盖到的, 除非它是整页的倍数
    for (; pdir_idx < pdir_idx_end; pdir_idx ++) {
      // fill PDE
      kpdirs[pdir_idx] = (uintptr_t)ptab | PTE_P;

      // fill PTE
      PTE pte = PGADDR(pdir_idx, 0, 0) | PTE_P;
      PTE pte_end = PGADDR(pdir_idx + 1, 0, 0) | PTE_P;
      for (; pte < pte_end; pte += PGSIZE) {
        *ptab = pte;
        ptab ++;
      }
    }
  }

  set_cr3(kpdirs); // 我觉得这里也该设置一下cur_as
  set_cr0(get_cr0() | CR0_PG);
  vme_enable = 1;

  return 0;
}

// 用于创建一个默认的地址空间
int _protect(_AddressSpace *as) {
  if(vme_enable) {
    PDE *updir = (PDE*)(pgalloc_usr(1));
    as->ptr = updir;
    // map kernel space
    for (int i = 0; i < NR_PDE; i ++) {
      updir[i] = kpdirs[i];
    }
  }

  return 0;
}

void _unprotect(_AddressSpace *as) {
}

static _AddressSpace *cur_as = NULL;
void __am_get_cur_as(_Context *c) {
  c->as = cur_as;
}

// 切换到另一个_Context的as
void __am_switch(_Context *c) {
  if (vme_enable) {
    set_cr3(c->as->ptr); // 这说明as->ptr就是一级页表的地址
    // Log_debug("cr3:%x", c->as->ptr);
    cur_as = c->as;
  }
}

// 创建映射, 在as中使va映射到pa
int _map(_AddressSpace *as, void *vaddr, void *paddr, int prot) {
  (void)prot;
  addr_t addr;
  addr.val = vaddr;
  PDE* pde = as->ptr + (addr.hi << 2);
  if ((*pde & PTE_P)==0) {
    void* tmp_paddr = pgalloc_usr(1);
    // Log_debug("allocate for pte: paddr=%x, vaddr=%x", (size_t)tmp_paddr, (size_t)vaddr);
    *pde = (size_t)tmp_paddr | PTE_P;
  }
  PTE* pte = (*pde&0xfffff000)+(addr.mid << 2);
  if ((*pte & PTE_P)==0) {
    *pte = (size_t)paddr | PTE_P;
  } else {
  }
  // Log_debug("vaddr:%x, paddr:%x", (size_t)vaddr, (size_t)paddr);
  return 0;
}

void* add_vmap(_AddressSpace *as, size_t va) {
  if (vme_enable) {
    void* ret = has_map(as, va);
    if(ret == NULL) {
      void *pa = pgalloc_usr(1);
      // Log_debug("allocate: paddr=%x, vaddr=%x, cr3=%x", (size_t)pa, (size_t)va, (size_t)as->ptr);
      _map(as, PTE_ADDR(va), pa, 0);
      ret = (void*)((size_t)pa | OFF(va));
    }
    return ret;
  } else {
    return NULL;
  }
}

// 右闭
int add_vmap_range(_AddressSpace *as, void *va_start, void* va_end) {
  if (vme_enable) {
    void* addr_down = PGROUNDDOWN((size_t)va_start);
    void* addr_up = PGROUNDDOWN((size_t)va_end);
    void* cur_addr = addr_down;
    while(cur_addr<=addr_up) {
      add_vmap(as, cur_addr);
      cur_addr+=PGSIZE;
    }
  }
}

size_t has_map(_AddressSpace *as, size_t vaddr) {
  if (vme_enable) {
    addr_t addr;
    addr.val = vaddr;
    PDE* pde = as->ptr + (addr.hi << 2);
    if ((*pde & PTE_P)==0) {
      return 0;
    }
    PTE* pte = (*pde&0xfffff000)+(addr.mid << 2);
    if ((*pte & PTE_P)==0) {
      return 0;
    }
    return (*pte & 0xfffff000) | addr.lo;
  } else {
    return 0;
  }
}

// start是虚拟地址, end是物理地址
_Context *_ucontext(_AddressSpace *as, _Area ustack, _Area kstack, void *entry, char* argv[], char* envp[]) {
  int n_envp = 0;
  while(envp[n_envp]!=0) {
    ++n_envp;
  }

  int argc = 0;
  while(argv[argc]!=0) {
    ++argc;
  }

  size_t cur_pos = ustack.end;
  for(int i = n_envp-1; i >= 0; --i) {
    size_t len = strlen(envp[i])+1;
    Log_debug("envp, i=%d,len=%d", i, len);
    cur_pos -= len;
    strcpy((char*)cur_pos, envp[i]);
  }
  size_t envp_base = cur_pos;
  size_t tmp_envp_base = cur_pos;
  Log_debug("before argv");

  for(int i = argc-1; i >= 0; --i) {
    size_t len = strlen(argv[i])+1;
    Log_debug("argv, i=%d,len=%d", i, len);
    cur_pos -= len;
    strcpy((char*)cur_pos, argv[i]);
  }
  size_t argv_base = cur_pos;
  size_t tmp_argv_base = cur_pos;

  size_t* envp_addr_base = cur_pos - sizeof(size_t)*(1+n_envp);
  envp_addr_base[n_envp] = NULL;
  for(int i = 0; i < n_envp; ++i) {
    envp_addr_base[i] = tmp_envp_base;
    tmp_envp_base += strlen((char*)tmp_envp_base)+1;
  }

  size_t* argv_addr_base = (size_t)envp_addr_base - sizeof(size_t)*(1+argc);
  argv_addr_base[argc] = NULL;
  for(int i = 0; i < argc; ++i) {
    argv_addr_base[i] = tmp_argv_base;
    tmp_argv_base += strlen((char*)tmp_argv_base)+1;
  }

  size_t* esp = argv_addr_base - 3*sizeof(size_t);
  esp[0] = argc;
  esp[1] = argv_addr_base;
  esp[2] = envp_addr_base;

  // 相比_kcontext, 大概需要设置GPRx
  _Context* context = (_Context*)(kstack.end - sizeof(_Context));
  memset(context, 0, sizeof(_Context));
  context->as = as;
  // Log_debug("as:%x", (size_t)as->ptr);
  // 由于popa中并没有用到esp, 因此不需要设置esp
  context->eip = entry;
  context->cs = 8;
  assert((size_t)esp >= (size_t)ustack.end - PGSIZE);
  context->esp = (size_t)esp + (size_t)ustack.start - (size_t)ustack.end -4; // start是虚拟地址, end是物理地址. -4是返回地址
  context->eflags |= FL_IF; // 保证开中断
	// printf("[ucontext]argc:%d, argv:%x, envp:%x, argv[0](%x):%s, argv[1](%x):%s\n", argc, argv_addr_base, envp_addr_base, argv_addr_base[0], argv_addr_base[0], argv_addr_base[1], argv_addr_base[1]);
  return context;
}

bool is_kernel_thread(_Context* c) {
  return (c->as->ptr == kpdirs);
}
