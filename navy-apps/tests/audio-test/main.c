#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

extern void NDL_Init(int);
extern void NDL_OpenAudio(int, int, int);
extern void NDL_PlayAudio(uint8_t *, int);
extern int NDL_QueryAudio();

int main() {
  //FILE  *f = fopen("/share/music/little-star.ogg", "r");
  FILE  *f = fopen("/share/music/rhythm/Do.ogg", "r");
  fseek(f, 0, SEEK_END);
  int len = ftell(f);
  uint8_t *buf = malloc(len);
  fseek(f, 0, SEEK_SET);
  assert(len == fread(buf, 1, len, f));

  
  NDL_Init(0);
  printf("buf is %p, len is %d\n", buf, len);
  NDL_OpenAudio(8000, 1, 1024);
  NDL_PlayAudio(buf, len);
  int rest = 0;
  while ((rest = NDL_QueryAudio()) == 0) {
    printf("rest = %d\n", rest);
  }
  free(buf);
}
