#include <common.h>

#if defined(MULTIPROGRAM) && !defined(TIME_SHARING)
# define MULTIPROGRAM_YIELD() yield()
#else
# define MULTIPROGRAM_YIELD()
#endif

#define NAME(key) \
  [AM_KEY_##key] = #key,

static const char *keyname[256] __attribute__((used)) = {
  [AM_KEY_NONE] = "NONE",
  AM_KEYS(NAME)
};

size_t serial_write(const void *buf, size_t offset, size_t len) {
  char *p = (char *)buf;
  for (size_t i = 0; i < len; i++) {
    putch(p[i]);
  }
  return len;
}

size_t events_read(void *buf, size_t offset, size_t len) {
  AM_INPUT_KEYBRD_T ev = io_read(AM_INPUT_KEYBRD);
  if(ev.keycode == AM_KEY_NONE)
  {
    *(char*)buf = '\0';
    return 0;
  }
  return snprintf((char*)buf, len, "%s %s\n", ev.keydown ? "kd" : "ku", keyname[ev.keycode]);
}

size_t dispinfo_read(void *buf, size_t offset, size_t len) {
  // print into buf
  AM_GPU_CONFIG_T cfg = io_read(AM_GPU_CONFIG);
  // printf("cfg.width is %d, cfg.height is %d\n", cfg.width, cfg.height);
  return snprintf(buf, len, "WIDTH: %d\nHEIGHT: %d\n", cfg.width, cfg.height);
}

size_t fb_write(const void *buf, size_t offset, size_t len) {
  AM_GPU_CONFIG_T ev = io_read(AM_GPU_CONFIG);
  // 0 and 512
  offset /= 4;
  len /= 4;

  int x = offset % ev.width;
  int y = offset / ev.width;
  io_write(AM_GPU_FBDRAW, x, y, (char*)buf, len, 1, true);
  return len;
}

size_t sb_write(const void *buf, size_t offset, size_t len) {
  io_write(AM_AUDIO_PLAY, (Area){.start = (void *)buf, .end = (void *)buf + len});

  return len;
}

size_t sbctl_write(const void *buf, size_t offset, size_t len) {
  AM_AUDIO_CTRL_T ctrl = *(AM_AUDIO_CTRL_T *)buf;
  io_write(AM_AUDIO_CTRL, ctrl.freq, ctrl.channels, ctrl.samples);
  return len;
}

void init_device() {
  Log("Initializing devices...");
  ioe_init();
}
