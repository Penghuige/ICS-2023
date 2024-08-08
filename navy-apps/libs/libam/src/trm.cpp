#include <am.h>
#include <assert.h>

# define nemu_trap(code) asm volatile("mv a0, %0; ebreak" : :"r"(code))

Area heap;

void putch(char ch) {
  putchar(ch);
}

void halt(int code) {
  nemu_trap(code);

  // should not reach here
  while(1);
}
