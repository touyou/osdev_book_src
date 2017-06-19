/*
 *            framebuffer.c
 * written by Shinichi Awamoto, 2017
 * 
 * 画面描画関連の関数の定義
 */

#include "framebuffer.h"
#include "common.h"

static struct multiboot_tag_framebuffer_common *framebuffer_tag;

void framebuffer_init(struct multiboot_tag_framebuffer_common *tag) {
  if (tag == NULL) {
    panic();
  }
  framebuffer_tag = tag;
}

// char文字の描画
static void framebuffer_putc(char c) {
  // TODO
}

// https://github.com/swetland/xv6 より引用
/* The xv6 software is: */

/* Copyright (c) 2006-2009 Frans Kaashoek, Robert Morris, Russ Cox, */
/*                         Massachusetts Institute of Technology */

/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the */
/* "Software"), to deal in the Software without restriction, including */
/* without limitation the rights to use, copy, modify, merge, publish, */
/* distribute, sublicense, and/or sell copies of the Software, and to */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions: */

/* The above copyright notice and this permission notice shall be */
/* included in all copies or substantial portions of the Software. */

/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND */
/* NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE */
/* LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION */
/* OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION */
/* WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

static char digits[] = "0123456789abcdef";

static void
framebuffer_printptr(uint64_t x) {
  int i;
  for (i = 0; i < (sizeof(uint64_t) * 2); i++, x <<= 4)
    framebuffer_putc(digits[x >> (sizeof(uint64_t) * 8 - 4)]);
}

static void
framebuffer_printint(int xx, int base, int sign)
{
  char buf[16];
  int i;
  uint32_t x;

  if(sign && (sign = xx < 0))
    x = -xx;
  else
    x = xx;

  i = 0;
  do{
    buf[i++] = digits[x % base];
  }while((x /= base) != 0);

  if(sign)
    buf[i++] = '-';

  while(--i >= 0)
    framebuffer_putc(buf[i]);
}

void
framebuffer_printf(char *fmt, ...)
{
  va_list ap;
  int i, c;
  char *s;

  va_start(ap, fmt);

  if (fmt == 0)
    panic("null fmt");

  for(i = 0; (c = fmt[i] & 0xff) != 0; i++){
    if(c != '%'){
      framebuffer_putc(c);
      continue;
    }
    c = fmt[++i] & 0xff;
    if(c == 0)
      break;
    switch(c){
    case 'd':
      framebuffer_printint(va_arg(ap, int), 10, 1);
      break;
    case 'x':
      framebuffer_printint(va_arg(ap, int), 16, 0);
      break;
    case 'p':
      framebuffer_printptr(va_arg(ap, uint64_t));
      break;
    case 's':
      if((s = va_arg(ap, char*)) == 0)
        s = "(null)";
      for(; *s; s++)
        framebuffer_putc(*s);
      break;
    case '%':
      framebuffer_putc('%');
      break;
    default:
      // Print unknown % sequence to draw attention.
      framebuffer_putc('%');
      framebuffer_putc(c);
      break;
    }
  }
}
