#include <am.h>
#include <NDL.h>
#include "amdev.h"
#include <assert.h>

int w, h;

bool ioe_init() {
  NDL_Init(0);
  NDL_OpenCanvas(&w, &h);
  return true;
}

void ioe_read (int reg, void *buf) {
  // support some io read
  printf("reg is %d\n", reg);
  switch (reg) {
    case AM_TIMER_CONFIG: 
      break;
  }
  assert(0);
}
void ioe_write(int reg, void *buf) {
  assert(0);
}
