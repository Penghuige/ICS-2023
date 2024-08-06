#include <NDL.h>
#include <SDL.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>


#define keyname(k) #k,

static const char *keyname[] = {
  "NONE",
  _KEYS(keyname)
};

int SDL_PushEvent(SDL_Event *ev) {
  assert(0);
  return 0;
}


int SDL_PeepEvents(SDL_Event *ev, int numevents, int action, uint32_t mask) {
  assert(0);
  return 0;
}

uint8_t* SDL_GetKeyState(int *numkeys) {
  assert(0);
  return NULL;
}

int SDL_WaitEvent(SDL_Event *ev) {
  while(SDL_PollEvent(ev) == 0); 
  return 1;
}

#define MIN(a, b) (a < b ? a : b)
int SDL_PollEvent(SDL_Event *ev) {
  char* buf = malloc(4096);
  // read out the event
  if(NDL_PollEvent(buf, 20))
  {
    int sign = 0;
    // it has a event, judge it.
    if(strncmp(buf, "kd", 2) == 0)
    {
      ev->type = SDL_KEYDOWN;
    }
    else
    {
      ev->type = SDL_KEYUP;
    }
    // keyname is different from buf[3], and it length is different;
    // if not do this, the slides will not up, only down
    for (int i = 0; i < sizeof(keyname) / sizeof(keyname[0]); i++) {
      // when the length is bigger than 2, it maybe a bug
      if (strncmp(buf + 3, keyname[i], MIN(strlen(keyname[i]) , 4)) == 0){
        // it isn't long key such as t between tab
        if(strlen(buf+3) != 1)
        {
          //printf("get buf is %s, while keyname is %s\n", buf+3, keyname[i]);
          continue;
        }
         ev->key.keysym.sym = i;
         sign = 1;
         break;
      }
    }
    if(!sign)
    {
      printf("keyname: %s\n", buf);
      printf("strlen is %ld\n", strlen(buf));
    }
    free(buf);
    return 1;
  }
  else
  {
    // if not do this, the slides will tranfer quickly!
    ev->key.type = SDL_USEREVENT; // avoid too many `Redirecting file open ...`
    ev->key.keysym.sym = 0;
  }
  free(buf);
  return 0;
}
