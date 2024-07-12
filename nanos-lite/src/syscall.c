#include <common.h>
#include "syscall.h"

void sys_exit(int code);
int sys_yield();
void sys_write(intptr_t *buf, size_t count);

void do_syscall(Context *c) {
  printf("now pc is %d", c->mepc);
  uintptr_t a[4];
  a[0] = c->GPR1;
  a[1] = c->GPR2;
  a[2] = c->GPR3;
  a[3] = c->GPR4;

//#ifdef CONFIG_STRACE
  printf("syscall ID = %d\n", a[0]);
//#endif

  // a[0] is a7 is the syscall ID, while a[1] is a0 is the syscall argument
  // when it call a sys_write, the syscall ID is 4, and the argument is the file descriptor
  switch (a[0]) {
    case 0: // sys_exit
      c->GPRx = 0;
      sys_exit(c->GPRx);
      break;
    case 1: // sys_yield
      sys_yield();
      // return value is zero
      c->GPRx = 0;
      break;
    case 4: // sys_write
      //sys_write((intptr_t*)a[2], a[3]);
      printf("%s", (char*)a[2]);
      c->GPRx = a[3];
      break;
    default: panic("Unhandled syscall ID = %d", a[0]);
  }
}

void sys_exit(int code) {
  halt(code);
}

int sys_yield() {
  asm volatile("li a7, 0; ecall");
  //yield();

  return 0;
}

void sys_write(intptr_t *buf, size_t count){
  for (int i = 0; i < count; i++) {
    putch(*((char*)buf + i));
  }
}
