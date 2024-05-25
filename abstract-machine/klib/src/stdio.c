#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

int printf(const char *fmt, ...) {
  panic("Not implemented");
}

int vsprintf(char *out, const char *fmt, va_list ap) {
  panic("Not implemented");
}

int sprintf(char *out, const char *fmt, ...) {
  size_t i;
  size_t j = 0;

  va_list ap;
  char *s;
  int d;
  char buff[20];
  size_t t;

  va_start(ap, fmt);
  
  for(i = 0; fmt[i] != '\0'; i++)
  {
    if(fmt[i] == '%')
    {
      i++;
      switch (fmt[i])
      {
        case '%':
          out[j++] = '%';
          break;
        case 's':
          s = va_arg(ap, char *);
          for(t = 0; s[t] != '\0'; t++)
          {
            out[j++] = s[t];
          }
          break;
        case 'd':
          d = va_arg(ap, int);
          //itoa(d, buff, 10);
          t = 0;
          if(d < 0)
          {
            buff[t++] = '-';
          }
          else if(d == 0)
          {
            buff[t++] = '0'; 
          }
          while(d != 0)
          {
            buff[t++] = d % 10 + '0';
            d /= 10;
          }
          size_t tmp = (d < 0) ? 1 : 0;
          for(tmp = 0; tmp < t / 2; tmp++)
          {
            char c = buff[t-tmp-1];
            buff[t-tmp-1] = buff[tmp];
            buff[tmp] = c;
          }
          buff[t] = '\0';

          for(t = 0; buff[t] != '\0'; t++)
          {
            out[j++] = buff[t];
          }
          break;
      }
    }
    else
    {
      out[j++] = fmt[i];
    }
  }
  out[j] = '\0';
  return strlen(out);
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  panic("Not implemented");
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  panic("Not implemented");
}

#endif
