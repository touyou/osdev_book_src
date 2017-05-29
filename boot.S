#define ASM_FILE
#include "multiboot2.h"

#define MULTIBOOTHEADER_SIZE (multibootheader_end - multibootheader_start)

.global entry
.global multiboot_info
.global gdt_desc
.global gdtr
.code32
.section .bootstrap

.balign 8

.extern initial_stack_buffer
.extern initial_PML4T
  
.extern main
multibootheader_start:
  .long MULTIBOOT2_HEADER_MAGIC
  .long MULTIBOOT_ARCHITECTURE_I386
  .long MULTIBOOTHEADER_SIZE
  .long -(MULTIBOOT2_HEADER_MAGIC + MULTIBOOT_ARCHITECTURE_I386 + MULTIBOOTHEADER_SIZE)

  // multiboot tags
  // 8バイト境界にアラインされていること
  // 参考： multiboot2 specification 3.1.3 General tag structure

  .balign 8
align_tag:  
  // 参考： multiboot2 specification 3.1.11 Module alignment tag
  .short MULTIBOOT_HEADER_TAG_MODULE_ALIGN
  .short 0
  .long 8

  .balign 8
framebuffer_tag:  
  // 参考： multiboot2 specification 3.1.10 The framebuffer tag of Multiboot2 header
  .short MULTIBOOT_HEADER_TAG_FRAMEBUFFER
  .short 0
  .long  20
  .long 1024
  .long 768
  .long 32

  .balign 8
end_tag:
  .short MULTIBOOT_HEADER_TAG_END
  .short 0
  .long 8
  
multibootheader_end:

.text
entry:
  cli
  hlt
