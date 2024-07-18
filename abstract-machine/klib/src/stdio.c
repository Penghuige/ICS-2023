#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

static char sprint_buf[1024];

int printf(const char *fmt, ...)
{
  va_list args; 
  int n;
  va_start(args, fmt);
  n = vsprintf(sprint_buf, fmt, args);
  va_end(args);
  putstr(sprint_buf);
  return n;
}

int my_printf(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  char buff[20] = {0};
  
  size_t i;
  size_t j = 0;

  char *s;
  char c;
  int d;
  size_t t;

  for(i = 0; fmt[i] != '\0'; i++)
  {
    if(fmt[i] == '%')
    {
      i++;
      switch (fmt[i])
      {
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
            //out[j++] = buff[t];
            putch(buff[t]);
          }
          break;
        case 's':
          s = va_arg(ap, char *);
          for(t = 0; s[t] != '\0'; t++)
          {
            //out[j++] = s[t];
            putch(s[t]);
          }
          break;
        case 'c':
          c = va_arg(ap, int);
          putch(c);
          break;
        default:
          break;
      }
    }
    else
    {
      //out[j++] = fmt[i];
      putch(fmt[i]);
    }
  }
  //out[j] = '\0';
  

  //for(i = 0; out[i] != '\0'; i++)
  //{
  //  putch(out[i]);
  //}
  va_end(ap);
  return (int)j;
}

int vsprintf(char *out, const char *fmt, va_list ap) {
  size_t i;
  size_t j = 0;

  char *s;
  int d;
  char buff[20];
  char c;
  size_t t;
  
  for(i = 0; fmt[i] != '\0'; i++)
  {
    if(fmt[i] == '%')
    {
      i++;
      switch (fmt[i])
      {
        case 's':
          s = va_arg(ap, char *);
          for(t = 0; s[t] != '\0'; t++)
          {
            out[j++] = s[t];
          }
          break;
        case 'x':
          d = va_arg(ap, int);
          char hex[17] = {"0123456789abcdef"};
          char hex_num[10] = {0};
          int k = 0;
          if(d == 0)
          {
            out[j++] = '0';
          }
          else
          {
            while(d != 0)
            {
              hex_num[k++] = hex[d % 16];
              d /= 16;
            }
            out[j++] = '0';
            out[j++] = 'x';
            for(k--; k >= 0; k--)
            {
              out[j++] = hex_num[k];
            }
          }
          break;
        case 'd':
          d = va_arg(ap, int);
          //itoa(d, buff, 10);
          t = 0;
          size_t tmp = (d < 0) ? 1 : 0;
          if(d < 0)
          {
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
          if(tmp == 1) buff[t++] = '-';
          for(tmp = 0 ; tmp < t / 2; tmp++)
          {
            char c = buff[t-tmp-1];
            buff[t-tmp-1] = buff[tmp];
            buff[tmp] = c;
          }
          buff[t++] = '\0';

          for(t = 0; buff[t] != '\0'; t++)
          {
            out[j++] = buff[t];
          }
          break;
        case 'c':
          c = va_arg(ap, int);
          out[j++] = c; 
          break;
        default:
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
  char c;

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
        case 'x':
          int d = va_arg(ap, int);
          char hex[17] = "0123456789abcdef";
          char hex_num[10] = {0};
          int k = 0;
          if (d == 0) {
            out[j++] = '0';
          } else {
            while (d != 0) {
              hex_num[k++] = hex[d % 16];
              d /= 16;
            }
            out[j++] = '0';
            out[j++] = 'x';
            for (k--; k >= 0; k--) {
              out[j++] = hex_num[k];
            }
          }
          break;
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
        case 'c':
          c = va_arg(ap, int);
          putch(c);
          break;
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
  char c;
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
        case 'c':
          c = va_arg(ap, int);
          putch(c);
          break;
        case 'x':
          d = va_arg(ap, int);
          char hex[17] = {"0123456789abcdef"};
          char hex_num[10] = {0};
          int k = 0;
          if(d == 0)
          {
            out[j++] = '0';
          }
          else
          {
            while(d != 0)
            {
              hex_num[k++] = hex[d % 16];
              d /= 16;
            }
            out[j++] = '0';
            out[j++] = 'x';
            for(k--; k >= 0; k--)
            {
              out[j++] = hex_num[k];
            }
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
        case 'x':
          d = va_arg(ap, int);
          char hex[17] = {"0123456789abcdef"};
          char hex_num[10] = {0};
          int k = 0;
          if(d == 0)
          {
            out[j++] = '0';
          }
          else
          {
            while(d != 0)
            {
              hex_num[k++] = hex[d % 16];
              d /= 16;
            }
            out[j++] = '0';
            out[j++] = 'x';
            for(k--; k >= 0; k--)
            {
              out[j++] = hex_num[k];
            }
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
