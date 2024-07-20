#include <stdio.h>
#include <assert.h>

extern int NDL_PollEvent(char *buf, int len);

int main()
{
  char* s;
  NDL_PollEvent(s, 10);
  while(1)
  {
    printf("%s", s);
    NDL_PollEvent(s, 10);
  }

  return 0;
}
