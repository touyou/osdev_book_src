/*
 *            common.h
 * written by Shinichi Awamoto, 2017
 * 
 * よく使われる定義や関数宣言のまとめ
 */

#pragma once

typedef __INT8_TYPE__ int8_t;
typedef __UINT8_TYPE__ uint8_t;
typedef __INT16_TYPE__ int16_t;
typedef __UINT16_TYPE__ uint16_t;
typedef __INT32_TYPE__ int32_t;
typedef __UINT32_TYPE__ uint32_t;
typedef __INT64_TYPE__ int64_t;
typedef __UINT64_TYPE__ uint64_t;

typedef __builtin_va_list va_list;

#define va_start(v,l)   __builtin_va_start(v,l)
#define va_end(v)       __builtin_va_end(v)
#define va_arg(v,l)     __builtin_va_arg(v,l)

#define panic() while(1){__asm__ volatile("cli;hlt;nop;");}

#define MASK(val, ebit, sbit) ((val) & (((1 << ((ebit) - (sbit) + 1)) - 1) << (sbit)))

enum ReturnCode {
  SUCCESS,
  ERROR,
};

#define NULL ((void *)0)

static inline void outb(int pin, uint8_t data) {
  __asm__ volatile("outb %%al, %%dx"::"d"(pin), "a"(data));
}

uint8_t font[128][8];
