#include <common.h>
#include "am.h"
#include <sys/time.h>
#include "syscall.h"
#include <proc.h>

void sys_exit(int code);
int sys_yield();
int sys_write(int fd, intptr_t *buf, size_t count);
int sys_gettimeofday(uintptr_t *a);
int sys_execve(const char *fname, char * const argv[], char *const envp[]);

extern int fs_open(const char *pathname, int flags, int mode);
extern size_t fs_read(int fd, intptr_t *buf, size_t count);
extern size_t fs_write(int fd, intptr_t *buf, size_t count);
extern int fs_close(int fd);
extern size_t fs_lseek(int fd, size_t offset, int whence);

extern void naive_uload(PCB *pcb, const char *filename);

void do_syscall(Context *c) {
  uintptr_t a[4];
  a[0] = c->GPR1;
  a[1] = c->GPR2;
  a[2] = c->GPR3;
  a[3] = c->GPR4;

//#ifdef CONFIG_STRACE
//  printf("syscall ID = %d\n", a[0]);
//#endif

  // a[0] is a7 is the syscall ID, while a[1] is a0 is the syscall argument
  // when it call a sys_write, the syscall ID is 4, and the argument is the file descriptor
  // end is used to store the program break
  extern char end;
  switch (a[0]) {
    case SYS_exit: // sys_exit
      sys_exit(a[1]);
      c->GPRx = 0;
      break;
    case SYS_yield: // sys_yield
      yield();
      // return value is zero
      c->GPRx = 0;
      break;
    case SYS_open: // sys_open
      //printf("path: %s\n", "hallo?");
      c->GPRx = fs_open((char*)a[1], a[2], a[3]);
      break;
    case SYS_read: // sys_read
      c->GPRx = fs_read(a[1], (intptr_t*)a[2], a[3]);
      break;
    case SYS_write: // sys_write
      c->GPRx = sys_write(a[1], (intptr_t*)a[2], a[3]);
      //printf("%s", (char*)a[2]);
      break;
    case SYS_close: // sys_close
      c->GPRx = fs_close(a[1]);
      break;
    case SYS_lseek: // sys_lseek
      c->GPRx = fs_lseek(a[1], a[2], a[3]);
      break;
    case SYS_brk: // sys_brk
      end = c->GPR2;
      c->GPRx = 0;
      break;
    case SYS_execve: // SYS_execve
      c->GPRx = sys_execve((char*)a[1], (char**)a[2], (char**)a[3]);
      break;
    case SYS_gettimeofday: // sys_gettimeofday
      c->GPRx = sys_gettimeofday(a);
      break;
    default: panic("Unhandled syscall ID = %d", a[0]);
  }
}

void sys_exit(int code) {
  halt(0);
}

int sys_yield() {
  //asm volatile("li a7, 0; ecall");
  yield();
  return 0;
}

int sys_read(int fd, intptr_t *buf, size_t count) {
  return fs_read(fd, buf, count);
}

int sys_write(int fd, intptr_t *buf, size_t count){
  return fs_write(fd, buf, count);
}

int sys_gettimeofday(uintptr_t *a) {
  struct timeval* tv = (struct timeval*)a[1];
  struct timezone* tz = (struct timezone*)a[2];
  uint64_t us = io_read(AM_TIMER_UPTIME).us;
  if (tv != NULL) {
    tv->tv_sec = us / (1000*1000);
    tv->tv_usec = us % (1000*1000);
  }
  else
  {
    Log("tv is NULL!");
  }
  if (tz != NULL) {
    // to implement
  }
  return 0;
}

int sys_execve(const char *fname, char * const argv[], char *const envp[]) {
  // need to clear the now status?
  naive_uload(NULL, fname);
  
  return -1;
}
