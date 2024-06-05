#include <am.h>
#include <nemu.h>

#define AUDIO_FREQ_ADDR      (AUDIO_ADDR + 0x00)
#define AUDIO_CHANNELS_ADDR  (AUDIO_ADDR + 0x04)
#define AUDIO_SAMPLES_ADDR   (AUDIO_ADDR + 0x08)
#define AUDIO_SBUF_SIZE_ADDR (AUDIO_ADDR + 0x0c)
#define AUDIO_INIT_ADDR      (AUDIO_ADDR + 0x10)
#define AUDIO_COUNT_ADDR     (AUDIO_ADDR + 0x14)

static uint32_t nplay = 0;

void __am_audio_init() {
  outl(AUDIO_FREQ_ADDR, 0);
  outl(AUDIO_CHANNELS_ADDR, 0);
  outl(AUDIO_SAMPLES_ADDR, 0);
}

void __am_audio_config(AM_AUDIO_CONFIG_T *cfg) {
  cfg->present = true;
  cfg->bufsize = inl(AUDIO_SBUF_SIZE_ADDR);
}


void __am_audio_ctrl(AM_AUDIO_CTRL_T *ctrl) {
  // here is out, not in.
  outl(AUDIO_FREQ_ADDR, ctrl->freq);
  outl(AUDIO_SAMPLES_ADDR, ctrl->samples);
  outl(AUDIO_CHANNELS_ADDR, ctrl->channels);
  // tell the nemu, it needs init 
  outl(AUDIO_INIT_ADDR, 1);
}

void __am_audio_status(AM_AUDIO_STATUS_T *stat) {
  stat->count = inl(AUDIO_COUNT_ADDR);
}


void __am_audio_play(AM_AUDIO_PLAY_T *ctl) {
  uint8_t* audio_payload = ctl->buf.start;
  uint8_t* audio_payload_end = ctl->buf.end;
  uint32_t len = &audio_payload_end - &audio_payload;

  uint32_t sbuf_size = inl(AUDIO_SBUF_ADDR);
  uint8_t * audio_data = ctl->buf.start;
  
  uint8_t *ab = (uint8_t *)(uintptr_t)AUDIO_SBUF_ADDR;  //参考GPU部分
  for(int i = 0; i < len; i++){
    ab[nplay] = audio_data[i];
    nplay = (nplay + 1) % sbuf_size;  
  }
}
