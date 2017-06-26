/*
 *            segment_asm.S
 * written by Shinichi Awamoto, 2017
 * 
 * segment, TSSの設定
 */


  #define ASM_FILE
  #include "segment.h"

  .global lgdt_sub

lgdt_sub:
  pushq $KERNEL_CS
  pushq $jmp                    
  lgdt (%rdi)                   
  lretq                         
jmp:
  movl $KERNEL_DS, %eax
  mov %ax, %ds
  mov %ax, %es
  mov %ax, %fs
  mov %ax, %gs
  mov %ax, %ss
  ret
