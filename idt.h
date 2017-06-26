/*
 *            idt.h
 * written by Shinichi Awamoto, 2017
 * 
 * IDTの設定
 */

#pragma once

#include "common.h"

// 割り込みハンドラが起動される直前のレジスタ状態
// nはvector番号
struct regs {
  uint64_t rax, rbx, rcx, rdx, rbp, rsi, rdi, r8, r9, r10, r11, r12, r13, r14, r15;
  uint64_t n, ecode, rip, cs, rflags;
  uint64_t rsp, ss;
}__attribute__((__packed__));

typedef void (*idt_callback)(struct regs *rs);

void idt_init();
void idt_init_for_each_proc();
void idt_register_callback(int n, idt_callback func);
