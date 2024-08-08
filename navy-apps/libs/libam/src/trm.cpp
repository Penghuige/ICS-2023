#include <am.h>
#include <assert.h>
#include <stdlib.h>

# define nemu_trap(code) asm volatile("mv a0, %0; ebreak" : :"r"(code))

Area heap;

void putch(char ch) {
  putchar(ch);
}

void halt(int code) {
  exit(code);

  // should not reach here
  while(1);
}
