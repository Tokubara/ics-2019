#include "common.h"
#include "proc.h"

int check_function_key();
static _Context* do_event(_Event e, _Context* c) {
  _Context* ret = NULL;
  switch (e.event) {
    case _EVENT_IRQ_TIMER: {
                             // Log_trace("timer interrupt");
                             --current->ticks;
                             if (current->ticks == 0) {
                               Log_trace("timer yield");
                               current->ticks = current->priority;
                               int fn_key = check_function_key();
                               if (fn_key > 0) {
                                 Log_trace("switch to %d", fn_key);
                                 fg_pcb = &pcb[fn_key];
                               }
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
