/*
  Simplified library of function calls to provide basic STDIO capabilities
  in a baremetal Altera Arria 10 SoC environment
  
  by  David M. Koltak  02/22/2016

***

Copyright (c) 2016 David M. Koltak

Permission is hereby granted, free of charge, to any person obtaining a copy of 
this software and associated documentation files (the "Software"), to deal in the 
Software without restriction, including without limitation the rights to use, 
copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in 
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
  
*/

#include "simple_stdio.h"
#include <string.h>
#include <stdarg.h>
#include "alt_16550_uart.h"

// NOTE: This handle must be initialized before using stdio gets/puts/printf/etc.
//       If not initialized, stdio will be silently dropped/skipped
ALT_16550_HANDLE_t _stdio_uart_handle =
{
  .device = -1,
  .location = (void*)0,
  .clock_freq = 0,
  .data = 0,
  .fcr = 0,
};

void chomp(char *s)
{
  char *ptr;
  ptr = s;
  
  while (*ptr != '\0')
    ptr++;
  
  while (*ptr <= ' ')
  {
    *ptr = '\0';
    
    if (ptr == s)
      break;
      
    ptr--;
  }
  
  return;
}

void flush()
{
  uint32_t level;
  uint32_t status;

  if (_stdio_uart_handle.device < 0)
    return;
    
  do
  {
    alt_16550_fifo_level_get_tx(&_stdio_uart_handle, &level);
  } while (level > 0);

  do
  {
    alt_16550_line_status_get(&_stdio_uart_handle, &status);
  } while ((status & ALT_16550_LINE_STATUS_TEMT) == 0);
  
  return;
}

int getchar(void)
{
  int rtn;
  uint32_t level;
  char buf;

  if (_stdio_uart_handle.device < 0)
    return 0;
      
  level = 0;
  do {
    alt_16550_fifo_level_get_rx(&_stdio_uart_handle, &level);
  } while (level <= 0);
  
  alt_16550_fifo_read(&_stdio_uart_handle, &buf, 1);
  
  rtn = (int) buf;
  return rtn;
}

char *safe_gets(char *s, int max)
{
  char *ptr;
  uint32_t level;

  if (_stdio_uart_handle.device < 0)
  {
    if (max)
      *s = '\0';
  
    return s;
  }
      
  ptr = s;
  level = 0;
  
  if (max == 0)
    return s;
  
  max--;
  
  while (max > 0)
  {
    while (level <= 0)
    { alt_16550_fifo_level_get_rx(&_stdio_uart_handle, &level); }
  
    alt_16550_fifo_read(&_stdio_uart_handle, ptr, 1);
    level--;
    
    if ((*ptr == 10) || (*ptr == 13)) // CR,LF
    {
      alt_16550_fifo_write(&_stdio_uart_handle, ptr, 1);
      ptr++;
      break;
    }
    else if ((*ptr == 8) || (*ptr == 127))  // backspace
    {
      if (ptr != s)
      {
        alt_16550_fifo_write(&_stdio_uart_handle, "\b \b", 3);
        ptr--;
        max++;
      }
    }
    else
    {
      alt_16550_fifo_write(&_stdio_uart_handle, ptr, 1);
      ptr++;
      max--;
    }
  }
  
  *ptr = '\0';
  return s;
}

char *gets(char *s)
{
  return safe_gets(s, 128);
}

int putchar(int c)
{
  uint32_t level;
  uint32_t fsize;
  char buf;

  if (_stdio_uart_handle.device < 0)
    return 0;
      
  alt_16550_fifo_size_get_tx(&_stdio_uart_handle, &fsize);
  
  do {
    alt_16550_fifo_level_get_tx(&_stdio_uart_handle, &level);
  } while (level >= fsize);
  
  buf = (char) (c & 0xFF);
  
  alt_16550_fifo_write(&_stdio_uart_handle, &buf, 1);
  return c;
}

int puts(char *s)
{
  char *ptr;
  uint32_t level;
  uint32_t fsize;
  int rtn;

  if (_stdio_uart_handle.device < 0)
    return 0;
      
  alt_16550_fifo_size_get_tx(&_stdio_uart_handle, &fsize);
  ptr = s;
  level = fsize;
  rtn = 0;
  
  while (1)
  {
    while (level >= fsize)
    { alt_16550_fifo_level_get_tx(&_stdio_uart_handle, &level); }
  
    if (*ptr == '\0')
      break;
    
    alt_16550_fifo_write(&_stdio_uart_handle, ptr, 1);
      
    level++;
    ptr++;
    rtn++;
  }
  
  alt_16550_fifo_write(&_stdio_uart_handle, "\n", 1);
  rtn++;
  
  return rtn;
}

int va_sprintf(char *s, char *f, va_list args)
{
  char *s_ptr;
  char *f_ptr;
  int rtn;
  int right_justify;
  int zero_pad;
  int field_width;
  int long_arg;
  char num_buf[32];
  char *str;
  int len;
  int inum;
  unsigned int unum;
  int x, y;
  
  s_ptr = s;
  f_ptr = f;
  rtn = 0;
  
  while (*f_ptr != '\0')
  {
    if (*f_ptr == '%')
    {
      f_ptr++;
      right_justify = 0;
      zero_pad = 0;
      field_width = 0;
      long_arg = 0;
      
      if (*f_ptr == '%')
      {
        *s_ptr = *f_ptr;
        f_ptr++;
        s_ptr++;
        continue;
      }
      
      if (*f_ptr == '-')
      {
        right_justify = 1;
        f_ptr++;
      }
      
      while (*f_ptr == '0')
      {
        zero_pad = 1;
        f_ptr++;
      }
      
      while ((*f_ptr >= '0') && (*f_ptr <= '9'))
      {
        field_width *= 10;
        field_width += *f_ptr - '0';
        f_ptr++;
      }

      while ((*f_ptr == 'l') || (*f_ptr == 'L'))
      {
        long_arg++;
        f_ptr++;
      }
            
      if ((*f_ptr == 'd') || (*f_ptr == 'i'))
      {
        inum = va_arg(args, int);
        if (inum == 0)
          str = "0";
        else
        {
          str = num_buf;
          if (inum < 0)
          {
            inum = -inum;
            *str = '-';
            str++;
          }
          x = 1000000000;
          while (x > 0)
          {
            y = inum / x;
            inum -= (y * x);
            x = x / 10;
            *str = ((char) (y & 0xF)) + '0';
            str++;
          }
          *str = '\0';
          str = num_buf;
          if (*str == '-')
          {
            str++;
            while (*str == '0')
              str++;
            str--;
            *str = '-';
          }
          else
          {
            while (*str == '0')
              str++;
          }
        }
        f_ptr++;
      }
      else if (*f_ptr == 'u')
      {
        unum = va_arg(args, unsigned int);
        if (unum == 0)
          str = "0";
        else
        {
          str = num_buf;
          x = 1000000000;
          while (x > 0)
          {
            y = unum / x;
            unum -= (y * x);
            x = x / 10;
            *str = ((char) (y & 0xF)) + '0';
            str++;
          }
          *str = '\0';
          str = num_buf;
          while (*str == '0')
            str++;
        }
        f_ptr++;
      }
      else if ((*f_ptr == 'x') || (*f_ptr == 'X') || (*f_ptr == 'p'))
      {
        if (*f_ptr == 'p')
        {
          field_width = 8;
          zero_pad = 1;
          unum = (unsigned int) va_arg(args, void *);
        }
        else
          unum = va_arg(args, unsigned int);
          
        if (unum == 0)
          str = "0";
        else
        {
          str = num_buf;
          for (x = 0; x < 8; x++)
          {
            *str = (char) ((unum >> 28) & 0xF);
            *str += '0';
            if (*str > '9')
            {
              *str += 7;
              if (*f_ptr == 'x')
                *str += 0x20;
            }
            str++;
            unum = (unum << 4);            
          }
          *str = '\0';
          str = num_buf;
          while (*str == '0')
            str++;
        }
        f_ptr++;
      }
      else if (*f_ptr == 's')
      {
        str = va_arg(args, char *);
        f_ptr++;
      }
      else
      {
        str = "%BAD%";
        f_ptr++;
      }
      
      if (long_arg)
      { str = "%BAD%"; }
      
      len = strlen(str);
      if (right_justify || zero_pad)
      {
        while (len < field_width)
        {
          *s_ptr = (zero_pad) ? '0' : ' ';
          s_ptr++;
          rtn++;
          len++;
        }
      }
      while (*str != '\0')
      {
        *s_ptr = *str;
        s_ptr++;
        rtn++;
        str++;
      }
      while (len < field_width)
      {
        *s_ptr = ' ';
        s_ptr++;
        rtn++;
        len++;
      }
    }
    else if (*f_ptr != '\0')
    {
      *s_ptr = *f_ptr;
      f_ptr++;
      s_ptr++;
      rtn++;
    }
  }
  
  *s_ptr = '\0';
  return rtn;
}

int sprintf(char *s, char *f, ...)
{
  va_list args;
  va_start(args, f);
  
  return va_sprintf(s, f, args);  
}

int printf(char *f, ...)
{
  char *ptr;
  uint32_t level;
  uint32_t fsize;
  int rtn;
  char buf[256];
  va_list args;
  va_start(args, f);

  if (_stdio_uart_handle.device < 0)
    return 0;
    
  va_sprintf(buf, f, args);
  
  alt_16550_fifo_size_get_tx(&_stdio_uart_handle, &fsize);
  ptr = buf;
  level = fsize;
  rtn = 0;
  
  while (*ptr != '\0')
  {
    while (level >= fsize)
    { alt_16550_fifo_level_get_tx(&_stdio_uart_handle, &level); }
  
    alt_16550_fifo_write(&_stdio_uart_handle, ptr, 1);
    level++;
    ptr++;
    rtn++;
  }
  
  return rtn;
}

int simple_is_hexdigit(char *s)
{
  if ((*s >= '0') && (*s <= '9'))
    return 1;
  
  if ((*s >= 'A') && (*s <= 'F'))
    return 1;
    
  if ((*s >= 'a') && (*s <= 'f'))
    return 1;
    
  return 0;
}

char* simple_parse_number(char *s, int width, int *is_neg, unsigned int *n)
{
  unsigned int unum;
  char *s_ptr;
  int base;
  int x;
  
  unum = 0;
  *is_neg = 0;
  s_ptr = s;
  
  while ((*s_ptr < '0') || (*s_ptr > '9'))
  {
    if (*s_ptr == '\0')
      break;
    
    if (*s_ptr == '-')
      *is_neg = 1;
    else
      *is_neg = 0;
      
    s_ptr++;
  }
  if (*s_ptr != '0')
    base = 10;
  else if (*s_ptr != '\0')
  {
    s_ptr++;
    if ((*s_ptr == 'x') || (*s_ptr == 'X'))
    {
      base = 16;
      s_ptr++;
    }
    else
      base = 8;
  }       
  while (simple_is_hexdigit(s_ptr) && (width > 0))
  {
    x = (*s_ptr - '0');
    if (x > 9)
      x = (x - 7) & 0xF;
    if (x >= base)
      break;
    unum *= base;
    unum += x;
    s_ptr++;
    width--;
  }
  
  *n = unum;
  return s_ptr;
}

int sscanf(char *s, char *f, ...)
{
  char *s_ptr;
  char *f_ptr;
  int field_width;
  int long_arg;
  int inum;
  int is_neg;
  unsigned int unum;
  int base;
  char *str;
  int rtn;
  
  va_list args;
  va_start(args, f);
  
  s_ptr = s; 
  f_ptr = f;
  rtn = 0;

  while (*f_ptr != '\0')
  {
    while (*s_ptr == ' ')
      s_ptr++;
      
    while (*f_ptr == ' ')
      f_ptr++;
  
    if (*f_ptr == '%')
    {
      f_ptr++;
      field_width = 0;
      long_arg = 0;
      
      while ((*f_ptr >= '0') && (*f_ptr <= '9'))
      {
        field_width *= 10;
        field_width += *f_ptr - '0';
        f_ptr++;
      }

      while ((*f_ptr == 'l') || (*f_ptr == 'L'))
      {
        long_arg++;
        f_ptr++;
      }
      
      if ((*f_ptr == 'd') || (*f_ptr == 'i'))
      {
        if (field_width == 0)
          field_width = 8;

        s_ptr = simple_parse_number(s_ptr, field_width, &is_neg, &unum);
        
        inum = (int) unum;
        if (is_neg)
          inum = -inum;
          
        *va_arg(args, int *) = inum;
        rtn++;
        f_ptr++;
      } 
      else if (*f_ptr == 'u')
      {
        if (field_width == 0)
          field_width = 8;
        
        s_ptr = simple_parse_number(s_ptr, field_width, &is_neg, &unum);
          
        *va_arg(args, unsigned int *) = unum;
        rtn++;
        f_ptr++;
      }
      else if ((*f_ptr == 'x') || (*f_ptr == 'X'))
      {
        unum = 0;
        while ((*s_ptr < '0') || (*s_ptr > 'f'))
        {
          if (*s_ptr == '\0')
            break;
          s_ptr++;
        }  
        while (simple_is_hexdigit(s_ptr) && (field_width > 0))
        {
          unum *= 16;
          inum = (*s_ptr - '0');
          if (inum > 9)
            inum = (inum - 7) & 0xF;
          unum += inum;
          s_ptr++;
          field_width--;
        }
        *va_arg(args, unsigned int *) = unum;
        rtn++;
        f_ptr++;
      }
      else if (*f_ptr == 's')
      {
        if (field_width == 0)
          field_width = 512;
        str = va_arg(args, char *);
        rtn++;
        f_ptr++;
        while ((*s_ptr != *f_ptr) && (field_width > 0))
        {
          *str = *s_ptr;
          if (*s_ptr == '\0')
            break;
          str++;
          s_ptr++;
          field_width--;
        }
        *str = '\0';
      }
    }
    else if (*s_ptr == *f_ptr)
    {
      s_ptr++;
      f_ptr++;
    }
    else
      break;
  }
  
  return rtn;
}
