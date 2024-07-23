#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <assert.h>
#include <fcntl.h> // Corrected header for file control options

static int evtdev = -1;
static int fbdev = -1;
static int screen_w = 0, screen_h = 0;

static int canvas_w = 0, canvas_h = 0;
static int canvas_x = 0, canvas_y = 0;

uint32_t NDL_GetTicks() {
  struct timeval tv;
  assert(gettimeofday(&tv, NULL) == 0);
  return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

int NDL_PollEvent(char *buf, int len) {
  int index = open("/dev/events", O_RDONLY); // Use O_RDONLY
  if (index < 0) {
    perror("Failed to open /dev/events");
    return 0;
  }
  int ret = read(index, buf, len);
  if (ret < 0) {
    perror("Failed to read from /dev/events");
    close(index);
    return 0;
  }
  assert(close(index) == 0);
  return ret > 0 ? 1 : 0;
}

void NDL_OpenCanvas(int *w, int *h) {
  int index = open("/proc/dispinfo", O_RDONLY);
  if (index < 0) {
    perror("Failed to open /proc/dispinfo");
    exit(1);
  }
  char buf[64];
  int nread = read(index, buf, sizeof(buf) - 1);
  if (nread < 0) {
    perror("Failed to read from /proc/dispinfo");
    close(index);
    exit(1);
  }
  buf[nread] = '\0'; // Null-terminate the buffer
  assert(close(index) == 0);

  sscanf(buf, "WIDTH:%d\nHEIGHT:%d", &screen_w, &screen_h);

  if (*w > screen_w || *h > screen_h) {
    fprintf(stderr, "Canvas size too large\n");
    exit(1);
  }
  if (*w == 0) {
    *w = screen_w;
  }
  if (*h == 0) {
    *h = screen_h;
  }
  canvas_w = *w;
  canvas_h = *h;
  canvas_x = (screen_w - canvas_w) / 2;
  canvas_y = (screen_h - canvas_h) / 2;

  if (getenv("NWM_APP")) {
    int fbctl = open("/dev/fbctl", O_WRONLY);
    fbdev = open("/dev/fb", O_RDWR);
    screen_w = *w;
    screen_h = *h;

    if (fbctl < 0 || fbdev < 0) {
      perror("Failed to open framebuffer device");
      exit(1);
    }

    char buf[64];
    int len = snprintf(buf, sizeof(buf), "%d %d", screen_w, screen_h);
    write(fbctl, buf, len);

    while (1) {
      int nread = read(evtdev, buf, sizeof(buf) - 1);
      if (nread <= 0) continue;
      buf[nread] = '\0';
      if (strcmp(buf, "mmap ok") == 0) break;
    }
    close(fbctl);
  }
}

void NDL_DrawRect(uint32_t *pixels, int x, int y, int w, int h) {
  int index = open("/dev/fb", O_WRONLY);
  if (index < 0) {
    perror("Failed to open /dev/fb");
    return;
  }
  lseek(index, ((canvas_y + y) * screen_w + canvas_x + x) * sizeof(uint32_t), SEEK_SET);
  for (int i = 0; i < h; i++) {
    write(index, pixels + i * w, w * sizeof(uint32_t));
    lseek(index, (screen_w - w) * sizeof(uint32_t), SEEK_CUR);
  }
  assert(close(index) == 0);
}

void NDL_OpenAudio(int freq, int channels, int samples) {
  // Not implemented yet
}

void NDL_CloseAudio() {
  // Not implemented yet
}

int NDL_PlayAudio(void *buf, int len) {
  return 0; // Not implemented yet
}

int NDL_QueryAudio() {
  return 0; // Not implemented yet
}

int NDL_Init(uint32_t flags) {
  if (getenv("NWM_APP")) {
    evtdev = open("/dev/events", O_RDONLY);
    if (evtdev < 0) {
      perror("Failed to open /dev/events");
      return -1;
    }
  }
  return 0;
}

void NDL_Quit() {
  if (evtdev >= 0) {
    close(evtdev);
  }
}

