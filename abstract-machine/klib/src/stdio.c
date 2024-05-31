#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)
int printf(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    char buff[20];
    
    size_t i = 0;
    size_t j = 0;

    char *s;
    char c;
    int d;
    size_t t;

    for (i = 0; fmt[i] != '\0'; i++) {
        if (fmt[i] == '%') {
            i++;
            switch (fmt[i]) {
                case 'd':
                    d = va_arg(ap, int);
                    if (d < 0) {
                        putch('-');
                        d = -d;
                        j++;
                    }
                    t = 0;
                    if (d == 0) {
                        buff[t++] = '0';
                    } else {
                        while (d != 0) {
                            buff[t++] = (d % 10) + '0';
                            d /= 10;
                        }
                        // Reverse the buffer
                        for (size_t k = 0; k < t / 2; k++) {
                            char tmp = buff[k];
                            buff[k] = buff[t - k - 1];
                            buff[t - k - 1] = tmp;
                        }
                    }
                    buff[t] = '\0';
                    for (size_t k = 0; buff[k] != '\0'; k++) {
                        putch(buff[k]);
                        j++;
                    }
                    break;
                case 's':
                    s = va_arg(ap, char *);
                    for (t = 0; s[t] != '\0'; t++) {
                        putch(s[t]);
                        j++;
                    }
                    break;
                case 'c':
                    c = va_arg(ap, int);
                    putch(c);
                    j++;
                    break;
                default:
                    putch('%');
                    putch(fmt[i]);
                    j += 2;
                    break;
            }
        } else {
            putch(fmt[i]);
            j++;
        }
    }
    va_end(ap);
    return (int)j;
}

int vsprintf(char *out, const char *fmt, va_list ap) {
  size_t i;
  size_t j = 0;

  char *s;
  int d;
  char buff[20];
  size_t t;
  
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
          size_t tmp = (d < 0) ? 1 : 0;
          if(d < 0)
          {
            buff[t++] = '-';
            d = -d;
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
          for( ; tmp < t / 2; tmp++)
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
int sprintf(char *out, const char *fmt, ...) {
  size_t i = 0;
  size_t j = 0;

  va_list ap;
  va_start(ap, fmt);

  while (fmt[i] != '\0') {
    if (fmt[i] == '%') {
      i++;
      switch (fmt[i]) {
        case '%':
          out[j++] = '%';
          break;
        case 's': {
          char *s = va_arg(ap, char *);
          while (*s) {
            out[j++] = *s++;
          }
          break;
        }
        case 'd': {
          int d = va_arg(ap, int);
          char buff[12]; // Enough to hold all digits of a 32-bit integer, including sign and null terminator
          int t = 0;
          if (d < 0) {
            out[j++] = '-';
            d = -d;
          }
          if (d == 0) {
            buff[t++] = '0';
          } else {
            while (d > 0) {
              buff[t++] = (d % 10) + '0';
              d /= 10;
            }
            for (int k = 0; k < t / 2; k++) {
              char temp = buff[k];
              buff[k] = buff[t - 1 - k];
              buff[t - 1 - k] = temp;
            }
          }
          buff[t] = '\0';
          for (int k = 0; buff[k] != '\0'; k++) {
            out[j++] = buff[k];
          }
          break;
        }
        default:
          // Unsupported format specifier
          out[j++] = '%';
          out[j++] = fmt[i];
          break;
      }
    } else {
      out[j++] = fmt[i];
    }
    i++;
  }
  out[j] = '\0';
  va_end(ap);
  return j;
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  size_t i;
  size_t j = 0;

  va_list ap;
  char *s;
  int d;
  char buff[20];
  size_t t;

  va_start(ap, fmt);
  
  for(i = 0; i < n && fmt[i] != '\0'; i++)
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
          size_t tmp = (d < 0) ? 1 : 0;
          if(d < 0)
          {
            buff[t++] = '-';
            d = -d;
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
          for( ; tmp < t / 2; tmp++)
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
  out[n] = '\0';
  va_end(ap);
  return strlen(out);
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  size_t i;
  size_t j = 0;

  char *s;
  int d;
  char buff[20];
  size_t t;

  
  for(i = 0; i < n && fmt[i] != '\0'; i++)
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
          size_t tmp = (d < 0) ? 1 : 0;
          if(d < 0)
          {
            buff[t++] = '-';
            d = -d;
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
          for( ; tmp < t / 2; tmp++)
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
  out[n] = '\0';
  return strlen(out);
}

#endif
