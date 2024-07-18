#include <common.h>

extern void do_syscall(Context* c);

static Context* do_event(Event e, Context* c) {
//#ifdef CONFIG_STRACE
  // the GPR1 is a7 register. It is used to pass the syscall number.  the GPRx is a0 register. It is used to pass the return value.
  Log("SYSCALL event ID=%d c->GPR1=%d c->GPR2=%d c->GPR3=%d c->GPRx=%d",e.event,c->GPR1, c->GPR2, c->GPR3, c->GPRx);
//#endif
  switch (e.event) {
    case EVENT_YIELD:
      break;
    case EVENT_SYSCALL:
      do_syscall(c);
      break;
    case EVENT_ERROR:
      printf("EVENT_ERROR\n");
    default: panic("Unhandled event ID = %d", e.event);
  }

  return c;
}

void init_irq(void) {
  Log("Initializing interrupt/exception handler...");
  cte_init(do_event);
}
