#include <common.h>
#include "syscall.h"

void sys_exit(int code);
void sys_yield();

void do_syscall(Context *c) {
  uintptr_t a[4];
  a[0] = c->GPR1;
  a[1] = c->GPR2;
  a[2] = c->GPR3;
  a[3] = c->GPR4;

//#ifdef CONFIG_STRACE
  printf("syscall ID = %d\n", a[1]);
//#endif

  switch (a[0]) {
    case EVENT_YIELD:
    case EVENT_SYSCALL:
      switch (a[1]) {
        case 0: // sys_exit
          sys_exit(a[2]);
          break;
        case 1: // sys_yield
          sys_yield();
          break;
        case 5: // sys_write
          printf("%.*s", a[3], (char *)a[2]);
          c->GPRx = a[3];
          break;
        default: panic("Unhandled syscall ID = %d", a[1]);
      }
      break;
      case 5: // sys_write
        printf("%.*s", a[3], (char *)a[2]);
        c->GPRx = a[3];
        break;
    default: panic("Unhandled syscall ID = %d", a[0]);
  }
}

void sys_exit(int code) {
  halt(code);
}

void sys_yield() {
  yield();
}
