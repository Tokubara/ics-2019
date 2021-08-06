#include <am.h>
#include <x86.h>
#include <debug.h>

static _Context* (*user_handler)(_Event, _Context*) = NULL;

void __am_irq0();
void __am_vecsys();
void __am_vectrap();
void __am_vecnull();

TSS tss;

size_t set_tss_esp0(size_t esp0) {
  size_t old_esp = tss.esp0;
  tss.esp0 = esp0;
  // Log_debug("old esp0: %x, new esp0: %x", old_esp, esp0);
  return old_esp;
}

_Context* __am_irq_handle(_Context *c) {
  __am_get_cur_as(c); // 设置c->as为cur_as
  _Context *next = c;
  if (user_handler) {
    _Event ev = {0};
    switch (c->irq) {
      case 32: {
                        ev.event = _EVENT_IRQ_TIMER;
                        break;
                      }
      case 0x80: {
                   ev.event = _EVENT_SYSCALL;
                   break;
                 }
      case 0x81: {
                   ev.event = _EVENT_YIELD;
                   break;
                 }
      default: ev.event = _EVENT_ERROR; break;
    }

    next = user_handler(ev, c);
    if (next == NULL) { // 如果是yield, 不为NULL
      next = c;
    }
  }
  __am_switch(next);
  return next;
}

int _cte_init(_Context*(*handler)(_Event, _Context*)) {
  static GateDesc idt[NR_IRQ];
  static SegDesc gdt[NR_SEG];

  // initialize IDT
  for (unsigned int i = 0; i < NR_IRQ; i ++) {
    idt[i] = GATE(STS_TG32, KSEL(SEG_KCODE), __am_vecnull, DPL_KERN); // 对GATE宏我们只需要关心第3个参数, 跳转的函数地址
  }

  // ----------------------- interrupts ----------------------------
  idt[32]   = GATE(STS_IG32, KSEL(SEG_KCODE), __am_irq0,   DPL_KERN);
  // ---------------------- system call ----------------------------
  idt[0x80] = GATE(STS_TG32, KSEL(SEG_KCODE), __am_vecsys, DPL_USER);
  idt[0x81] = GATE(STS_TG32, KSEL(SEG_KCODE), __am_vectrap, DPL_KERN);

  set_idt(idt, sizeof(idt));

  // register event handler
  user_handler = handler;

  // init GDT
  // 前面的没初始化, assert处理
  gdt[SEG_TSS] = SEG16(DESC_TYPE_TSS, &tss, sizeof(TSS), 0); // DPL本来也应该是, 这里再次设为0. TSS是系统段, G=0
  set_gdt(gdt, sizeof(gdt));

  // init TSS
  set_tr(SEG_TSS);
  set_tss_esp0(0); // 表示这是内核线程

  return 0;
}

_Context *_kcontext(_AddressSpace *as, _Area stack, void (*entry)(void *), void *arg) {
  _Context* context = (_Context*)(stack.end - sizeof(_Context) - 8);
  memset(context, 0, sizeof(_Context));
  context->as = as;
  // 由于popa中并没有用到esp, 因此不需要设置esp
  context->eip = entry;
  context->cs = 8;
  context->esp = 0;
  context->eflags |= FL_IF; // 保证开中断
  int* arg_addr = stack.end - 4; //? 这里能不能用void*?
  *arg_addr = arg;
  return context;
}

void _yield() {
  asm volatile("int $0x81");
}

int _intr_read() {
  return 0;
}

void _intr_write(int enable) {
}

