#include <am.h>
#include <x86.h>

static _Context* (*user_handler)(_Event, _Context*) = NULL;

void __am_irq0();
void __am_vecsys();
void __am_vectrap();
void __am_vecnull();

_Context* __am_irq_handle(_Context *c) {
  _Context *next = c;
  if (user_handler) {
    _Event ev = {0};
    switch (c->irq) {
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

  return next;
}

int _cte_init(_Context*(*handler)(_Event, _Context*)) {
  static GateDesc idt[NR_IRQ];

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

  return 0;
}

_Context *_kcontext(_Area stack, void (*entry)(void *), void *arg) {
  _Context* context = (_Context*)(stack.end - sizeof(_Context) - 8);
  memset(context, 0, sizeof(_Context));
  // 由于popa中并没有用到esp, 因此不需要设置esp
  context->eip = entry;
  context->cs = 8;
  int* arg_addr = stack.end - 4; //? 这里能不能用void*?
  *arg_addr = arg;
  // 或许可以
  // void* arg_addr = stack.end - 4;
  // memcpy(arg_addr, arg, 4);
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
