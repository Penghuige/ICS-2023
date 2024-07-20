#include <stdio.h>
#include <assert.h>
#include <sys/time.h>

extern int _gettimeofday(struct timeval *tv, struct timezone *tz);
extern uint32_t NDL_GetTicks();

//int main() {
//  struct timeval tv;
//  struct timeval pre;
//  _gettimeofday(&tv, NULL);
//  while(1)
//  {
//    pre = tv;
//    _gettimeofday(&tv, NULL);
//    __uint64_t ms = 500;
//    if(tv.tv_usec - pre.tv_usec > ms)
//    {
//      printf("hello! now is %ld not %ld\n", tv.tv_usec, pre.tv_usec);
//    }
//    printf("now is %ld not %ld\n", tv.tv_usec, pre.tv_usec);
//
//    printf("tv.tv_sec = %ld\n", tv.tv_sec);
//  }
//
//  return 0;
//}
//
int main()
{
  uint32_t t1 = NDL_GetTicks();
  uint32_t t2 = t1;
  while(1)
  {
    t2 = t1;
    t1 = NDL_GetTicks();
    // ms
    if(t1 - t2 > 500)
    {
      printf("i will be print!\n");
    }
  }

  return 0;
}
