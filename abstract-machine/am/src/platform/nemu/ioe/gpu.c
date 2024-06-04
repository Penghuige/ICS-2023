#include <am.h>
#include <nemu.h>

#define SYNC_ADDR (VGACTL_ADDR + 4)

#define N 32

void __am_gpu_init() {
  //uint32_t t = inl(VGACTL_ADDR);
  //int i;
  //int w = t >> 16;
  //int h = t & 0xffff;

  //outl(SYNC_ADDR, 1);
}

void __am_gpu_config(AM_GPU_CONFIG_T *cfg) {
  uint32_t t = inl(VGACTL_ADDR);
  int w = t >> 16;
  int h = t & 0xffff;
  *cfg = (AM_GPU_CONFIG_T) {
    .present = true, .has_accel = false,
    .width = w, .height = h,
    .vmemsz = 0
  };
}

void __am_gpu_fbdraw(AM_GPU_FBDRAW_T *ctl) {
  int x = ctl->x, y = ctl->y, w = ctl->w, h = ctl->h;
  if(!ctl->sync && (w == 0 || h == 0)) return;
  // get the screen width, avoid to leak
  uint32_t t = inl(VGACTL_ADDR) >> 16;

  uint32_t *p = ctl->pixels;
  uint32_t *fb = (uint32_t *)(uintptr_t)FB_ADDR;
  for (int i = y; i < y + h; i ++)
  {
    for(int j = x; j < x + w; j++)
    {
      fb[i*t+j] = p[(i-x)*t+(y-j)];
    }
  }
  if (ctl->sync) {
    outl(SYNC_ADDR, 1);
  }
}

void __am_gpu_status(AM_GPU_STATUS_T *status) {
  status->ready = true;
}
