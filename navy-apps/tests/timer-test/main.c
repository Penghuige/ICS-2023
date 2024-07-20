#include <stdio.h>
#include <assert.h>

struct timezone {
  int tz_minuteswest;
  int tz_dsttime;
};

extern int _gettimeofday(struct timeval *tv, struct timezone *tz);

int main() {
  struct timeval tv;
  struct timeval pre;
  _gettimeofday(&tv, NULL);
  while(1)
  {
    pre = tv;
    _gettimeofday(&tv, NULL);
    if(tv.tv_usec - pre.tv_usec == 500)
    {
      printf("hello! now is %ld not %ld\n", tv.tv_usec, pre.tv_usec);
    }
    printf(" now is %ld not %ld\n", tv.tv_usec, pre.tv_usec);

    printf("tv.tv_sec = %ld\n", tv.tv_sec);
  }
  printf("PASS!!!\n");

  return 0;
}
