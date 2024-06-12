#include <am.h>
#include <riscv/riscv.h>
#include <klib.h>

static Context* (*user_handler)(Event, Context*) = NULL;

Context* __am_irq_handle(Context *c) {
  if (user_handler) {
    for(int i = 0; i < 32; i++) printf("%u\n", c->gpr[i]);
    Event ev = {0};
    switch (c->mcause) {
      default: ev.event = EVENT_ERROR; break;
    }

    c = user_handler(ev, c);
    for(int i = 0; i < 32; i++) printf("%u\n", c->gpr[i]);
    assert(c != NULL);
  }

  return c;
}

extern void __am_asm_trap(void);

bool cte_init(Context*(*handler)(Event, Context*)) {
  // 内联汇编,异常处理的入口地址设置为__am_asm_trap。
  // %0 是内联汇编中的操作数占位符，它表示内联汇编指令中的第一个操作数。在这里的内联汇编指令中，%0 用来引用第一个输入操作数，即 "r"(__am_asm_trap) 中的 __am_asm_trap。
  //"r" 约束表示将一个寄存器作为输入操作数
  // initialize exception entry
  asm volatile("csrw mtvec, %0" : : "r"(__am_asm_trap));

  // register event handler
  user_handler = handler;

  return true;
}

Context *kcontext(Area kstack, void (*entry)(void *), void *arg) {
  return NULL;
}

void yield() {
#ifdef __riscv_e
  asm volatile("li a5, -1; ecall");
#else
  asm volatile("li a7, -1; ecall");
#endif
}

bool ienabled() {
  return false;
}

void iset(bool enable) {
}
