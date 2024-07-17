#include <common.h>
#include "syscall.h"

void sys_exit(int code);
int sys_yield();
void sys_write(intptr_t *buf, size_t count);

extern int fs_open(const char *pathname, int flags, int mode);
extern int fs_read(int fd, intptr_t *buf, size_t count);
extern int fs_close(int fd);
extern int fs_lseek(int fd, size_t offset, int whence);

void do_syscall(Context *c) {
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
  // end is used to store the program break
  extern char end;
  switch (a[0]) {
    case 0: // sys_exit
      sys_exit(0);
      c->GPRx = 0;
      break;
    case 1: // sys_yield
      sys_yield();
      // return value is zero
      c->GPRx = 0;
      break;
    case 2: // sys_open
      //path = "/share/files/num";
      //printf("path: %s\n", "hallo?");
      c->GPRx = fs_open((char*)a[1], a[2], a[3]);
      break;
    case 3: // sys_read
      c->GPRx = fs_read(a[1], (intptr_t*)a[2], a[3]);
      break;
    case 4: // sys_write
      sys_write((intptr_t*)a[2], a[3]);
      //printf("%s", (char*)a[2]);
      c->GPRx = a[3];
      break;
    case 7: // sys_close
      c->GPRx = fs_close(a[1]);
      break;
    case 8: // sys_lseek
      c->GPRx = fs_lseek(a[1], a[2], a[3]);
      break;
    case 9: // sys_brk
      end = c->GPR2;
      c->GPRx = 0;
      break;
    default: panic("Unhandled syscall ID = %d", a[0]);
  }
}

void sys_exit(int code) {
  halt(code);
}

int sys_yield() {
  //asm volatile("li a7, 0; ecall");
  yield();
  return 0;
}

int sys_read(int fd, intptr_t *buf, size_t count) {
  return fs_read(fd, buf, count);
}

void sys_write(intptr_t *buf, size_t count){
  for (int i = 0; i < count; i++) {
    putch(*((char*)buf + i));
  }
}
