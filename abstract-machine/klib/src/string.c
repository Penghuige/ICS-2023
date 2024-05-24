#include <klib.h>
#include <klib-macros.h>
#include <stdint.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *s) {
  size_t ret = 0;
  while(s[ret] != '\0')
  {
    ret++;
    assert(ret < 1000);
  }
  return ret;
}

char *strcpy(char *dst, const char *src) {
  size_t i = 0;
  while(src[i] != '\0')
  {
    dst[i] = src[i];
    i++;
  }
  dst[i] = '\0';
  return dst;
}

char *strncpy(char *dst, const char *src, size_t n) {
  size_t i;
  for(i = 0; i < n && src[i] != '\0'; i++)
    dst[i] = src[i];
  for( ; i < n; i++)
    dst[i] = '\0';
  return dst;
}

char *strcat(char *dst, const char *src) {
  size_t dest_len = strlen((const char*)src);
  size_t i;
  for(i = 0; src[i] != '\0'; i++)
    dst[dest_len + i] = src[i];
  dst[dest_len + i] = '\0';
  return dst;
}

int strcmp(const char *s1, const char *s2) {
  size_t i;
  for(i = 0; s1[i] != '\0' || s2[i] != '\0'; i++)
    if(s1[i] != s2[i]) return s1[i] - s2[i];
  if(s1[i] == '\0' && s2[i] == '\0') return 0;
  else if(s1[i] == '\0') return ' ' - s2[i];
  else return s1[i] - ' ';
}

int strncmp(const char *s1, const char *s2, size_t n) {
  panic("Not implemented");
}

void *memset(void *s, int c, size_t n) {
  panic("Not implemented");
}

void *memmove(void *dst, const void *src, size_t n) {
  panic("Not implemented");
}

void *memcpy(void *out, const void *in, size_t n) {
  panic("Not implemented");
}

int memcmp(const void *s1, const void *s2, size_t n) {
  panic("Not implemented");
}

#endif
