#include "common.h"
#include "proc.h"

static _Context* do_event(_Event e, _Context* c) {
  _Context* ret = NULL;
  switch (e.event) {
    case _EVENT_IRQ_TIMER: {
                             // Log_trace("timer interrupt");
                             --current->ticks;
                             if (current->ticks == 0) {
                               current->ticks = current->priority;
                               _yield();
                             }
                             break;
                           }
    case _EVENT_SYSCALL: {
                           ret = do_syscall(c);
                           break;
                         }
    case _EVENT_YIELD: {
                         LLog("event _yield\n"); 
                         ret = schedule(c);
                         break;
                       };
    default: panic("Unhandled event ID = %d", e.event);
  }

  return ret;
}

void init_irq(void) {
  Log("Initializing interrupt/exception handler...");
  _cte_init(do_event);
}
