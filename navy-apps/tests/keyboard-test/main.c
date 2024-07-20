#include <stdio.h>
#include <assert.h>

extern int NDL_PollEvent(char *buf, int len);

int main()
{
  while(1)
  {
    char s[64];
    if(NDL_PollEvent(s, sizeof(s)) != 0) printf("receive: %s\n", s);
    for(int i = 0; i < 1000000; i++) ;
  }

  return 0;
}
