#include <am.h>
#include <NDL.h>
#include "amdev.h"
#include <assert.h>
#include <string.h>

int w, h;

bool ioe_init() {
  NDL_Init(0);
  NDL_OpenCanvas(&w, &h);
  return true;
}

#define KEY_NAMES(x) #x,

static const char *keynames[] = {
  "NONE",
  AM_KEYS(KEY_NAMES)
};

void ioe_read (int reg, void *buf) {
  // give the reg value.
  // support some io read
  printf("ioe_read reg is %d\n", reg);
  switch (reg) {
    case AM_GPU_CONFIG:
      ((AM_GPU_CONFIG_T *)buf)->width = w;
      ((AM_GPU_CONFIG_T *)buf)->height = h;
      break;
    case AM_TIMER_UPTIME:
      ((AM_TIMER_UPTIME_T *)buf)->us = NDL_GetTicks() * 1000;
      break;
    case AM_TIMER_CONFIG: 
      ((AM_TIMER_CONFIG_T *)buf)->present = true;
      ((AM_TIMER_CONFIG_T *)buf)->has_rtc = true;
      break;
    case AM_INPUT_CONFIG:
      ((AM_INPUT_CONFIG_T *)buf)->present = true;
      break;
    case AM_INPUT_KEYBRD:
      // it need to get the keydown value.
      // not value.
      char ebuf[64];
      if(0 == NDL_PollEvent((char *)ebuf, sizeof(ebuf))) {
        *((AM_INPUT_KEYBRD_T *)buf) = (AM_INPUT_KEYBRD_T){.keydown=false, .keycode=0};
        break;
      }
      // find who has been pressed.
      bool keydown = ebuf[1] == 'd';
      int cnt = -1;
      for(int i = 0; i < sizeof(keynames)/sizeof(keynames[0]); i++) {
        if (strncmp(ebuf + 3, keynames[i], strlen(keynames[i])) == 0)
        {
          cnt = i;
          break;
        }
      }
      if(cnt == -1)
      {
        printf("ebuf is %s, key not fonud!\n", ebuf);
        assert(0);
      }
      ((AM_INPUT_KEYBRD_T *)buf)->keydown = keydown;
      ((AM_INPUT_KEYBRD_T *)buf)->keycode = cnt;
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
