#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <assert.h>
#include <sys/fcntl.h>

static int evtdev = -1;
static int fbdev = -1;
static int screen_w = 0, screen_h = 0;

static int canvas_w = 0, canvas_h = 0;
static int canvas_x = 0, canvas_y = 0;

uint32_t NDL_GetTicks() {
  struct timeval tv;
  assert(gettimeofday(&tv, NULL) == 0);
  // ms
  return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

int NDL_PollEvent(char *buf, int len) {
  int index = open("/dev/events", 0, 0);
  int ret = read(index, buf, len);
  assert(close(index) == 0);
  return ret == 0 ? 0 : 1;
}

void NDL_OpenCanvas(int *w, int *h) {
  // open a canvas with w*h
  int index = open("/proc/dispinfo", 0, 0);
  // canvas is a frame buffer
  char buf[64];
  int nread = read(index, buf, sizeof(buf) - 1);
  assert(nread < sizeof(buf) - 1);
  assert(close(index) == 0);
  assert(strncmp(buf, "WIDTH:", 6) == 0);

  int wi = 0, hi = 0;
  int i = 6;
  for(; buf[i] != '\n'; i++)
  {
    if(buf[i] >= '0' && buf[i] <= '9')
    {
      wi = wi * 10 + buf[i] - '0';
    }
  }
  assert(strncmp(buf+i+1, "HEIGHT:", 7) == 0);

  i += 7;
  for(; buf[i] != '\n'; i++)
  {
    if(buf[i] >= '0' && buf[i] <= '9')
    {
      hi = hi * 10 + buf[i] - '0';
    }
  }
  screen_h = hi;
  screen_w = wi;
  // if not set, initialize.
  if(*w == 0)
  {
    *w = screen_w;
  }
  if(*h == 0)
  {
    *h = screen_h;
  }
  // not right
  canvas_w = *w, canvas_h = *h;
  // mid
  canvas_x=(screen_w - canvas_w) / 2;
  canvas_y=(screen_h - canvas_h) / 2;

  if (getenv("NWM_APP")) {
    int fbctl = 4;
    fbdev = 5;
    screen_w = *w; screen_h = *h;
    //printf("screen_w is %d, screen_h is %d\n", screen_h, screen_w);
    char buf[64];
    int len = sprintf(buf, "%d %d", screen_w, screen_h);
    // let NWM resize the window and create the frame buffer
    write(fbctl, buf, len);
    while (1) {
      // 3 = evtdev
      int nread = read(3, buf, sizeof(buf) - 1);
      if (nread <= 0) continue;
      buf[nread] = '\0';
      if (strcmp(buf, "mmap ok") == 0) break;
    }
    close(fbctl);
  }
}

void NDL_DrawRect(uint32_t *pixels, int x, int y, int w, int h) {
  // write into /dev/fb
  int index = open("/dev/fb", 0, 0);
  lseek(index, (canvas_y * w + canvas_x) * 4, SEEK_SET);
  for(int i = 0; i < h; i++)
  {
    // write into canvas, then write into file.
    write(index, pixels + i*w, canvas_w*4);
    lseek(index, w*4, SEEK_CUR);
    //printf("write at %d\n", (int)((y + i) * w + x)*4);
    //for(int j = 0; j < w; j++) printf("write %d ", (int)pixels[i*w + j]);
  }
  assert(close(index) == 0);
}

void NDL_OpenAudio(int freq, int channels, int samples) {
}

void NDL_CloseAudio() {
}

int NDL_PlayAudio(void *buf, int len) {
  return 0;
}

int NDL_QueryAudio() {
  return 0;
}


int NDL_Init(uint32_t flags) {
  if (getenv("NWM_APP")) {
    evtdev = 3;
  }
  return 0;
}

void NDL_Quit() {
}
