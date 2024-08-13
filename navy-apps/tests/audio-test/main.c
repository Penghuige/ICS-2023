#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <NDL.h>

int main() {
  // this test have some bug, dont use it
  FILE  *f = fopen("/share/music/little-star.ogg", "r");
  //FILE  *f = fopen("/share/music/rhythm/Do.ogg", "r");
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
  rest = NDL_QueryAudio();
  while(1);
  while (rest > 0) {
    printf("rest = %d\n", rest);
    rest = NDL_QueryAudio();
  }
  free(buf);
}
