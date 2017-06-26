/*
 *            segment.c
 * written by Shinichi Awamoto, 2017
 * 
 * segment, TSSの設定
 */

#include "segment.h"
#include "apic.h"

struct tss_st {
  uint32_t reserved1;
  uint32_t rsp0l;
  uint32_t rsp0h;
  uint32_t rsp1l;
  uint32_t rsp1h;
  uint32_t rsp2l;
  uint32_t rsp2h;
  uint32_t reserved2;
  uint32_t reserved3;
  uint32_t ist1l;
  uint32_t ist1h;
  uint32_t ist2l;
  uint32_t ist2h;
  uint32_t ist3l;
  uint32_t ist3h;
  uint32_t ist4l;
  uint32_t ist4h;
  uint32_t ist5l;
  uint32_t ist5h;
  uint32_t ist6l;
  uint32_t ist6h;
  uint32_t ist7l;
  uint32_t ist7h;
  uint32_t reserved4;
  uint32_t reserved5;
  uint16_t reserved6;
  uint16_t iomap;
} __attribute__((packed)) __attribute__((aligned(8)));

static uint32_t gdt_desc[MAX_CPU][64];
struct tss_st tss[MAX_CPU];

int cpuid_cnt = 0;

uint8_t stack_buffer[4096 * 8 * MAX_CPU];

void lgdt_sub(uint64_t addr);

static void lgdt(uint64_t gdt_desc, int entry_num) {
  volatile uint32_t gdtr[3];
  gdtr[0] = (entry_num * 8 - 1) << 16;
  gdtr[1] = gdt_desc & 0xFFFFFFFF;
  gdtr[2] = (gdt_desc >> 32) & 0xFFFFFFFF;

  lgdt_sub((uint64_t)gdtr + 2);
}

void gdt_init() {
  int cpuid = __sync_fetch_and_add(&cpuid_cnt, 1);
  
  for (int i = 0; i < 4; i++) {
    gdt_desc[cpuid][i] = 0;
  }
  gdt_desc[cpuid][KERNEL_CS / sizeof(uint32_t)] = 0;
  gdt_desc[cpuid][KERNEL_CS / sizeof(uint32_t) + 1] = 0x00209a00;
  gdt_desc[cpuid][KERNEL_DS / sizeof(uint32_t)] = 0;
  gdt_desc[cpuid][KERNEL_DS / sizeof(uint32_t) + 1] = 0x00009200;

  uint64_t tss_vaddr = (uint64_t)&tss[cpuid];
  gdt_desc[cpuid][(TSS / sizeof(uint32_t))] =
    MASK((sizeof(struct tss_st) - 1), 15, 0) |
    (MASK(tss_vaddr, 16, 0) << 16);
  gdt_desc[cpuid][(TSS / sizeof(uint32_t)) + 1] =
    (MASK(tss_vaddr, 23, 16) >> 16) |
    (9 << 8) |
    (3 << 13) |
    (1 << 15) |
    MASK((sizeof(struct tss_st) - 1), 19, 16) |
    MASK(tss_vaddr, 31, 24);
  gdt_desc[cpuid][(TSS / sizeof(uint32_t)) + 2] = tss_vaddr >> 32;
  gdt_desc[cpuid][(TSS / sizeof(uint32_t)) + 3] = 0;

  struct tss_st *tss = (struct tss_st *)tss_vaddr;

  uint64_t stack_buffer_addr = (uint64_t)stack_buffer;
  
  uint64_t rsp0 = stack_buffer_addr + 4096 * 8 * cpuid + 4096 * 1;
  tss->rsp0l = rsp0;
  tss->rsp0h = rsp0 >> 32;

  uint64_t rsp1 = stack_buffer_addr + 4096 * 8 * cpuid + 4096 * 2;
  tss->rsp1l = rsp1;
  tss->rsp1h = rsp1 >> 32;

  uint64_t rsp2 = stack_buffer_addr + 4096 * 8 * cpuid + 4096 * 3;
  tss->rsp2l = rsp2;
  tss->rsp2h = rsp2 >> 32;

  uint64_t dfstack = stack_buffer_addr + 4096 * 8 * cpuid + 4096 * 4;
  tss->ist1l = dfstack;
  tss->ist1h = dfstack >> 32;

  uint64_t nmistack = stack_buffer_addr + 4096 * 8 * cpuid + 4096 * 5;
  tss->ist2l = nmistack;
  tss->ist2h = nmistack >> 32;

  uint64_t debugstack = stack_buffer_addr + 4096 * 8 * cpuid + 4096 * 6;
  tss->ist3l = debugstack;
  tss->ist3h = debugstack >> 32;

  uint64_t mcestack = stack_buffer_addr + 4096 * 8 * cpuid + 4096 * 7;
  tss->ist4l = mcestack;
  tss->ist4h = mcestack >> 32;

  uint64_t genstack = stack_buffer_addr + 4096 * 8 * cpuid + 4096 * 8;
  tss->ist5l = genstack;
  tss->ist5h = genstack >> 32;

  tss->iomap = sizeof(struct tss_st);  // no I/O permission map

  lgdt((uint64_t)&gdt_desc[cpuid], 6);
  __asm__ volatile("ltr %0;"::"r"((uint16_t)TSS));
}
