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


  switch (a[0]) {
    case EVENT_SYSCALL:
      switch (a[1]) {
        case 0: // sys_exit
          sys_exit(a[2]);
          break;
        case 1: // sys_yield
          sys_yield();
          break;
        default: panic("Unhandled syscall ID = %d", a[1]);
      }
      break;
    //case 0:
    //  sys_exit(c->GPR2);
    //  break;
    default: panic("Unhandled syscall ID = %d", a[0]);
  }
}

void sys_exit(int code) {
  halt(code);
}

void sys_yield() {
  yield();
}
