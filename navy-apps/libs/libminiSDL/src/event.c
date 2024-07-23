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
int SDL_WaitEvent(SDL_Event *event) {
  SDL_PollEvent(event);
  return 1;
}

int SDL_PollEvent(SDL_Event *ev) {
  unsigned buf_size = 32;
  char *buf = (char *)malloc(buf_size * sizeof(char));
  if (NDL_PollEvent(buf, buf_size) == 1) {
      if (strncmp(buf, "kd", 2) == 0) {
          ev->key.type = SDL_KEYDOWN;
      } else {
          ev->key.type = SDL_KEYUP;
      }

      for (unsigned i = 0; i < sizeof(keyname) / sizeof(keyname[0]); ++i) {
          if (strncmp(buf + 3, keyname[i], strlen(buf) - 4) == 0) {
              ev->key.keysym.sym = i;
              break;
          }
      }

      free(buf);
      return 1;
  } else {
      ev->key.type = SDL_USEREVENT; // avoid too many `Redirecting file open ...`
      ev->key.keysym.sym = 0;
  }

  free(buf);
  return 0;
}
