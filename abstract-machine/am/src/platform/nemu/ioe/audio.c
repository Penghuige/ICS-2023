#include <am.h>
#include <nemu.h>
#include <SDL2/SDL.h>

#define AUDIO_FREQ_ADDR      (AUDIO_ADDR + 0x00)
#define AUDIO_CHANNELS_ADDR  (AUDIO_ADDR + 0x04)
#define AUDIO_SAMPLES_ADDR   (AUDIO_ADDR + 0x08)
#define AUDIO_SBUF_SIZE_ADDR (AUDIO_ADDR + 0x0c)
#define AUDIO_INIT_ADDR      (AUDIO_ADDR + 0x10)
#define AUDIO_COUNT_ADDR     (AUDIO_ADDR + 0x14)

static volatile int nplay = 0;

void __am_audio_init() {
  outl(AUDIO_FREQ_ADDR, 0);
  outl(AUDIO_CHANNELS_ADDR, 0);
  outl(AUDIO_SAMPLES_ADDR, 0);
}

void __am_audio_config(AM_AUDIO_CONFIG_T *cfg) {
  cfg->present = true;
  cfg->bufsize = inl(AUDIO_SBUF_SIZE_ADDR);
}

static void audio_play(void *userdata, uint8_t *stream, int len)
{
  int nread = len;
  if(nplay < len) nread = nplay;
  int b = 0;
  while (b < nread) {
    uint8_t* t = io_read(AM_AUDIO_PLAY).buf.start;
    int i = -1;
    for(i = 0; i < nread; i++)
    {
      stream[i] = t[i];
    }
    int n = i;
    if (n > 0) b += n;
  }
}

void __am_audio_ctrl(AM_AUDIO_CTRL_T *ctrl) {
  SDL_AudioSpec s = {};
  s.format = AUDIO_S16SYS;  // 假设系统中音频数据的格式总是使用16位有符号数来表示
  s.freq = ctrl->freq;
  s.channels = ctrl->channels;
  s.samples = ctrl->samples;
  s.callback = audio_play;
  s.userdata = NULL;        // 不使用
  SDL_InitSubSystem(SDL_INIT_AUDIO);
  SDL_OpenAudio(&s, NULL);
  SDL_PauseAudio(0);
}

void __am_audio_status(AM_AUDIO_STATUS_T *stat) {
  stat->count = inl(AUDIO_COUNT_ADDR);
}


void __am_audio_play(AM_AUDIO_PLAY_T *ctl) {
  uint8_t audio_payload = ctl->buf.start 
  uint8_t audio_payload_end = ctl->buf.end;
  uint32_t audio_len = &audio_payload_end - &audio_payload;
  nplay = 0;
  Area sbuf;
  sbuf.start = &audio_payload;
  while (nplay < audio_len) {
    int len = (audio_len - nplay > 4096 ? 4096 : audio_len - nplay);
    sbuf.end = sbuf.start + len;
    // write into hardware
    io_write(AM_AUDIO_PLAY, sbuf);
    sbuf.start += len;
    nplay += len;
  }

  // wait until the audio finishes
  while (io_read(AM_AUDIO_STATUS).count > 0);
}
