/*
 *            bootap.S
 * written by Shinichi Awamoto, 2017
 * 
 * APのエントリポイント、BSPからの情報を用いてIA-32eへの移行
 */

#define ASM_FILE
#include "segment.h"

.text
.code16
.balign 4096

.globl entryothers
.globl stack_for_ap
.extern cmain_for_ap
.extern pml4t
.extern gdt_desc

entryothers:
  cli
  
  mov $pml4t, %edi
  mov %edi, %cr3

  // PAE&PGEビットを立てる
  movl %cr4, %eax
  orl  $(1 << 5 | 1 << 7), %eax
  movl %eax, %cr4

  movl $0xC0000080, %ecx
  rdmsr
  orl  $(1 << 8), %eax
  wrmsr
  
  // PG bitを立てる
  movl %cr0, %eax
  orl  $(1 << 31 | 1 << 0), %eax
  andl $(~(1 << 30 | 1 << 29)), %eax
  movl %eax, %cr0
  lgdt gdtr_for_ap
  ljmp $KERNEL_CS, $entry64_for_ap

.code64
entry64_for_ap:
  movl $KERNEL_DS, %eax
  movw %ax, %ds
  movw %ax, %es
  movw %ax, %fs
  movw %ax, %gs
  movw %ax, %ss
  movq (stack_for_ap), %rsp
  call cmain_for_ap

.data
  .balign 8
  .quad 0
  .word 0
gdtr_for_ap:
  .word 8*4 - 1
  .quad gdt_desc

stack_for_ap:
  .quad 0
