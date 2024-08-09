#include <NDL.h>
#include <SDL.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>

void (*callback)(void *userdata, uint8_t *stream, int len);
void *userdata;
uint8_t *audio_buf;
int callback_size;

uint32_t pre_time = 0;
int gap = 0;
int is_locked = 0;
bool audio_playing = 0;

void CallbackHelper()
{
  printf("call back helper!\n");
  uint32_t cur_time = SDL_GetTicks();
  if (pre_time == 0) {
    pre_time = cur_time;
  }
  int len = cur_time - pre_time;
  printf("the len is %d\n", len);
  printf("the gap is %d\n", gap);
  printf("the lock is %d\n", is_locked);
  if(len >= gap && !is_locked)
  {
    SDL_LockAudio();
    pre_time = cur_time;
    printf("the callback size is %d\n", callback_size);
    for(int i = 0; i < callback_size; i++)
    {
      printf("%d ", audio_buf[i]);
    }

    callback(userdata, audio_buf, callback_size);
    SDL_UnlockAudio();
  }
}

int SDL_OpenAudio(SDL_AudioSpec *desired, SDL_AudioSpec *obtained) {
  assert(desired != NULL);

  NDL_OpenAudio(desired->freq, desired->channels, desired->samples);
  callback = desired->callback;
  userdata = desired->userdata;
  assert(callback != NULL);
  // copy to obtained
  if (obtained) {
    memcpy(obtained, desired, sizeof(SDL_AudioSpec));
    desired = obtained;
  }
  // the size is sample number multple by channel number multple by 2
  desired->size = callback_size = desired->samples * sizeof(uint8_t);
  audio_buf = (uint8_t*)malloc(callback_size);
  // calculate the gap between each callback
  gap = 1000 * desired->samples / desired->freq;
  audio_playing = 1;
  return 0;
}

void SDL_CloseAudio() {
  audio_playing = 0;
  free(audio_buf);
  NDL_CloseAudio();
}

void SDL_PauseAudio(int pause_on) {
  printf("pause on is %d\n", pause_on);
  audio_playing = !pause_on;
}

void SDL_MixAudio(uint8_t *dst, uint8_t *src, uint32_t len, int volume) {
}

SDL_AudioSpec *SDL_LoadWAV(const char *file, SDL_AudioSpec *spec, uint8_t **audio_buf, uint32_t *audio_len) {
  return NULL;
}

void SDL_FreeWAV(uint8_t *audio_buf) {
}

void SDL_LockAudio() {
  is_locked = 1;
}

void SDL_UnlockAudio() {
  is_locked = 0;
}
