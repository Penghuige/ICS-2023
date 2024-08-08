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
  // give the reg value.
  // support some io read
  printf("ioe_read reg is %d\n", reg);
  switch (reg) {
    case AM_GPU_CONFIG:
      ((AM_GPU_CONFIG_T *)buf)->width = w;
      ((AM_GPU_CONFIG_T *)buf)->height = h;
      break;
    case AM_TIMER_CONFIG: 
      ((AM_TIMER_UPTIME_T *)buf)->us = NDL_GetTicks();
      break;
    default:
      assert(0);
  }
}
void ioe_write(int reg, void *buf) {
  printf("ioe_write reg is %d\n", reg);
  switch (reg) {
    case AM_GPU_FBDRAW:
      NDL_DrawRect(((AM_GPU_FBDRAW_T *)buf)->pixels, ((AM_GPU_FBDRAW_T *)buf)->x, ((AM_GPU_FBDRAW_T *)buf)->y, ((AM_GPU_FBDRAW_T *)buf)->w, ((AM_GPU_FBDRAW_T *)buf)->h);
      break;
    default:
      assert(0);
  }
}
