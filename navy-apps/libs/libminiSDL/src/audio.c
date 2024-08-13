#include <NDL.h>
#include <SDL.h>
#include <assert.h>
#include <stdint.h>
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
  //printf("call back helper!\n");
  uint32_t cur_time = SDL_GetTicks();
  if (pre_time == 0) {
    pre_time = cur_time;
  }
  int len = cur_time - pre_time;
  if(len >= gap && !is_locked)
  {
    SDL_LockAudio();
    pre_time = cur_time;
    //printf("the callback size is %d\n", callback_size);
    //for(int i = 0; i < callback_size; i++)
    //{
    //  printf("%d ", audio_buf[i]);
    //}
    //printf("go to callback : %p\n", callback);
    if(callback) callback(userdata, audio_buf, callback_size);
    NDL_PlayAudio(audio_buf, callback_size);
    
    SDL_UnlockAudio();
  }
}

int SDL_OpenAudio(SDL_AudioSpec *desired, SDL_AudioSpec *obtained) {
  assert(desired != NULL);

  NDL_OpenAudio(desired->freq, desired->channels, desired->samples);
  //printf("call back is %p\n", desired->callback);
  callback = desired->callback;
  userdata = desired->userdata;
  assert(callback != NULL);
  // copy to obtained
  if (obtained) {
    memcpy(obtained, desired, sizeof(SDL_AudioSpec));
    desired = obtained;
  }
  // the size is sample number multple by channel number multple by 2
  desired->size = callback_size = desired->channels * desired->samples * sizeof(uint16_t);
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
  audio_playing = !pause_on;
}

void SDL_MixAudio(uint8_t *dst, uint8_t *src, uint32_t len, int volume) {
  // mix the audio
  //printf("mix audio!\n");
  int16_t *d = (int16_t*)dst;
  int16_t *s = (int16_t*)src;
  for(int i = 0; i < len / 2; i++)
  {
    int32_t tmp = d[i] + s[i] * volume / SDL_MIX_MAXVOLUME;
    if(tmp > 32767)
    {
      d[i] = 32767;
    }
    else if(tmp < -32768)
    {
      d[i] = -32768;
    }
    else
    {
      d[i] = tmp;
    }
  }
}

SDL_AudioSpec *SDL_LoadWAV(const char *file, SDL_AudioSpec *spec, uint8_t **audio_buf, uint32_t *audio_len) {
  // load a wav file.
  if (file == NULL || spec == NULL || audio_buf == NULL || audio_len == NULL)
    return NULL;
  FILE *f = fopen(file, "r");
  if (!f)
    return NULL;
  struct WAVEHeader {
    char ChunkID[4];
    uint32_t ChunkSize;
    char Format[4];
    char Subchunk1ID[4];
    uint32_t Subchunk1Size;
    uint16_t AudioFormat;
    uint16_t NumChannels;
    uint32_t SampleRate;
    uint32_t ByteRate;
    uint16_t BlockAlign;
    uint16_t BitsPerSample;
    char Subchunk2ID[4];
    uint32_t Subchunk2Size;
  } header;
  if(fread(&header, sizeof(header), 1, f) != 1)
  {
    fclose(f);
    return NULL;
  }
  if (strncmp(header.ChunkID, "RIFF", 4) != 0 ||
      strncmp(header.Format, "WAVE", 4) != 0 ||
      strncmp(header.Subchunk1ID, "fmt ", 4) != 0 ||
      header.AudioFormat != 1 ||
      strncmp(header.Subchunk2ID, "data", 4) != 0)
  {
    fclose(f);
    return NULL;
  }
  // alloc memory for audio_buf and
  // read the audio data.
  *audio_len = header.Subchunk2Size;
  *audio_buf = (uint8_t*)malloc(*audio_len);
  if(fread(*audio_buf, *audio_len, 1, f) != 1)
  {
    fclose(f);
    return NULL;
  }
  fclose(f);
  spec->freq = header.SampleRate;
  spec->format = AUDIO_S16SYS;
  spec->channels = header.NumChannels;
  spec->samples = 1024;
  return spec;
}

void SDL_FreeWAV(uint8_t *audio_buf) {
  free(audio_buf);
}

void SDL_LockAudio() {
  is_locked = 1;
}

void SDL_UnlockAudio() {
  is_locked = 0;
}
