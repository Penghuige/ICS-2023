#include <stdio.h>
#include <assert.h>

struct timezone {
  int tz_minuteswest;
  int tz_dsttime;
};

extern int _gettimeofday(struct timeval *tv, struct timezone *tz);

int main() {
  struct timezone tz;
  struct timeval tv;
  _gettimeofday(&tv, &tz);
  while(1)
  {
    _gettimeofday(&tv, &tz);
    printf("tv.tv_sec = %ld\n", tv.tv_sec);
    printf("tv.tv_usec = %ld\n", tv.tv_usec);
  }
  printf("PASS!!!\n");

  return 0;
}
