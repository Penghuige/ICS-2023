#include <common.h>

extern void do_syscall(Context* c);

static Context* do_event(Event e, Context* c) {
//#ifdef CONFIG_STRACE
  // the GPR1 is a7 register. It is used to pass the syscall number.  the GPRx is a0 register. It is used to pass the return value.
  printf("event ID=%d c->GPR1=%d c->GPRx=%d\n",e.event,c->GPR1, c->GPRx);
//#endif
  switch (e.event) {
    case EVENT_YIELD:
      printf("EVENT_YIELD not ");
    case EVENT_SYSCALL:
      printf("EVENT_SYSCALL\n");
      do_syscall(c);
      break;
    case EVENT_ERROR:
      Log("EVENT_ERROR\n");
    default: panic("Unhandled event ID = %d", e.event);
  }

  return c;
}

void init_irq(void) {
  Log("Initializing interrupt/exception handler...");
  cte_init(do_event);
}
