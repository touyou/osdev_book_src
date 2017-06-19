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

// 以下のコードはxv6より引用
// https://pdos.csail.mit.edu/6.828/2016/xv6.html

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

static void
framebuffer_printint(int xx, int base, int sgn)
{
  static char digits[] = "0123456789ABCDEF";
  char buf[16];
  int i, neg;
  uint32_t x;

  neg = 0;
  if(sgn && xx < 0){
    neg = 1;
    x = -xx;
  } else {
    x = xx;
  }

  i = 0;
  do{
    buf[i++] = digits[x % base];
  }while((x /= base) != 0);
  if(neg)
    buf[i++] = '-';

  while(--i >= 0)
    framebuffer_putc(buf[i]);
}

// Print to the given fd. Only understands %d, %x, %p, %s.
void
framebuffer_printf(char *fmt, ...)
{
  char *s;
  int c, i, state;
  void *ap;

  state = 0;
  ap = (void*)&fmt + 1;
  for(i = 0; fmt[i]; i++){
    c = fmt[i] & 0xff;
    if(state == 0){
      if(c == '%'){
        state = '%';
      } else {
        framebuffer_putc(c);
      }
    } else if(state == '%'){
      if(c == 'd'){
        framebuffer_printint(*(uint32_t *)ap, 10, 1);
        ap++;
      } else if(c == 'x' || c == 'p'){
        framebuffer_printint(*(uint32_t *)ap, 16, 0);
        ap++;
      } else if(c == 's'){
        s = (char*)*(uint64_t *)ap;
        ap++;
        if(s == 0)
          s = "(null)";
        while(*s != 0){
          framebuffer_putc(*s);
          s++;
        }
      } else if(c == 'c'){
        framebuffer_putc(*(char *)ap);
        ap++;
      } else if(c == '%'){
        framebuffer_putc(c);
      } else {
        // Unknown % sequence.  Print it to draw attention.
        framebuffer_putc('%');
        framebuffer_putc(c);
      }
      state = 0;
    }
  }
}

