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
  printf("breakdown ID = %d\n", a[0]);
  printf("syscall ID = %d\n", a[1]);
//#endif

  // a[0] is a7 is the syscall ID, while a[1] is a0 is the syscall argument
  switch (a[0]) {
    case 0: // sys_exit
      sys_exit(a[2]);
      break;
    case 1: // sys_yield
      sys_yield();
      break;
    case 4: // sys_write
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
