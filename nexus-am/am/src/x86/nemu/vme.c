#include <am.h>
#include <x86.h>
#include <nemu.h>

#define PG_ALIGN __attribute((aligned(PGSIZE)))

static PDE kpdirs[NR_PDE] PG_ALIGN = {};
static PTE kptabs[(PMEM_SIZE + MMIO_SIZE) / PGSIZE] PG_ALIGN = {};
static void* (*pgalloc_usr)(size_t) = NULL;
static void (*pgfree_usr)(void*) = NULL;
static int vme_enable = 0;

static _Area segments[] = {      // Kernel memory mappings
  {.start = (void*)0,          .end = (void*)PMEM_SIZE},
  {.start = (void*)MMIO_BASE,  .end = (void*)(MMIO_BASE + MMIO_SIZE)}
};

#define NR_KSEG_MAP (sizeof(segments) / sizeof(segments[0]))

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
    uint32_t pdir_idx = (uintptr_t)segments[i].start / (PGSIZE * NR_PTE);
    uint32_t pdir_idx_end = (uintptr_t)segments[i].end / (PGSIZE * NR_PTE);
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

  set_cr3(kpdirs);
  set_cr0(get_cr0() | CR0_PG);
  vme_enable = 1;

  return 0;
}

int _protect(_AddressSpace *as) {
  PDE *updir = (PDE*)(pgalloc_usr(1));
  as->ptr = updir;
  // map kernel space
  for (int i = 0; i < NR_PDE; i ++) {
    updir[i] = kpdirs[i];
  }

  return 0;
}

void _unprotect(_AddressSpace *as) {
}

static _AddressSpace *cur_as = NULL;
void __am_get_cur_as(_Context *c) {
  c->as = cur_as;
}

void __am_switch(_Context *c) {
  if (vme_enable) {
    set_cr3(c->as->ptr);
    cur_as = c->as;
  }
}

int _map(_AddressSpace *as, void *va, void *pa, int prot) {
  return 0;
}

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
    cur_pos -= len;
    strcpy((char*)cur_pos, envp[i]);
  }
  size_t envp_base = cur_pos;
  size_t tmp_envp_base = cur_pos;

  for(int i = argc-1; i >= 0; --i) {
    size_t len = strlen(argv[i])+1;
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
  // 由于popa中并没有用到esp, 因此不需要设置esp
  context->eip = entry;
  context->cs = 8;
  context->GPRx = esp;
	printf("[ucontext]argc:%d, argv:%x, envp:%x, argv[0](%x):%s, argv[1](%x):%s\n", argc, argv_addr_base, envp_addr_base, argv_addr_base[0], argv_addr_base[0], argv_addr_base[1], argv_addr_base[1]);
  return context;
}
