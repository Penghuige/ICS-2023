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
static int sbdev = -1;
static int sbtdev = -1;
static int screen_w = 0, screen_h = 0;

static int canvas_w = 0, canvas_h = 0;
static int canvas_x = 0, canvas_y = 0;

#define MIN(x, y) ((x) < (y) ? (x) : (y))

uint32_t NDL_GetTicks() {
  struct timeval tv;
  assert(gettimeofday(&tv, NULL) == 0);
  // ms
  return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

int NDL_PollEvent(char *buf, int len) {
	memset(buf, 0, len);
  int ret = read(evtdev, buf, len);
  //printf("NDL_PollEvent, buf is %s, len is %d\n", buf, len);
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

  if(*w > screen_w || *h > screen_h)
  {
    //printf("*w is %d, *h is %d\n", *w, *h);
    //printf("screen_w is %d, screen_h is %d\n", screen_w, screen_h);
    fprintf(stderr, "Canvas size too large\n");
    exit(1);
  }
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
  // 128 * 128 screen is 300 * 400
  //printf("canvas_w is %d, canvas_h is %d\n", canvas_w, canvas_h);
  // mid
  canvas_x=(screen_w - canvas_w) / 2;
  canvas_y=(screen_h - canvas_h) / 2;

	// a file define NWM_APP, but now it needn't
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
  for (int i = 0; i < h; i++) {
    lseek(fbdev, ((canvas_y + y + i) * screen_w + x + canvas_x) * sizeof(uint32_t), SEEK_SET);
    write(fbdev, &pixels[i * w], w * sizeof(uint32_t));
  }
}
void NDL_DrawRect_false(uint32_t *pixels, int x, int y, int w, int h) {
  // write into /dev/fb
  lseek(fbdev, ((canvas_y + y) * screen_w + canvas_x + x) * sizeof(uint32_t), SEEK_SET);
  for(int i = 0; i < h; i++)
  {
    //printf("locate at %d\n", (canvas_y + y + i) * screen_w + canvas_x + x);
    // write into canvas, then write into file.
    write(fbdev, pixels + i*w, w*4);
    lseek(fbdev, canvas_w*4, SEEK_CUR);
    //printf("write at %d\n", (int)((y + i) * w + x)*4);
    //for(int j = 0; j < w; j++) printf("write %d ", (int)pixels[i*w + j]);
  }
}

void NDL_OpenAudio(int freq, int channels, int samples) {
  int buf[3] = {freq, channels, samples};
  printf("freq is %d, channels is %d, samples is %d\n", freq, channels, samples);
  write(sbtdev, buf, sizeof(buf));
}

void NDL_CloseAudio() {
}


int NDL_QueryAudio() {
  // read from the file and play the music
  // empty!
  // need to return the length of the unused buffer
  char buf[16] = {0};
  read(sbtdev, buf, sizeof(buf));
  return atoi(buf);
}

int NDL_PlayAudio(void *buf, int len) {
  return write(sbdev, buf, len);
}
//int NDL_PlayAudio(void *buf, int len) {
//  assert(buf != NULL);
//  int ret = len;
//  //assert(0);
//  int spare = NDL_QueryAudio();
//  while(len > 0)
//  {
//    //printf("NDL_QueryAudio() is %d\n", NDL_QueryAudio());
//    write(sbdev, buf, MIN(len, spare));
//    len -= spare;
//    spare = NDL_QueryAudio();
//    break;
//  }
//  return ret - len;
//}

int NDL_Init(uint32_t flags) {
  if (getenv("NWM_APP")) {
    evtdev = 3;
  }
  else
	{
    fbdev = open("/dev/fb", 0, 0);
    evtdev = open("/dev/events", 0, 0);
    sbdev = open("/dev/sb", 0, 0);
    sbtdev = open("/dev/sbctl", 0, 0);
  }

  if (fbdev == -1 || evtdev == -1 || sbdev == -1 || sbtdev == -1) {
      perror("Error initializing devices");
      return -1;
  }
  return 0;
}

void NDL_Quit() {
  assert(close(fbdev) == 0);
  assert(close(evtdev) == 0);
  assert(close(sbdev) == 0);
  assert(close(sbtdev) == 0);
}
