/*
 *            idt.c
 * written by Shinichi Awamoto, 2017
 * 
 * IDTの設定
 */

#include "apic.h"
#include "idt.h"
#include "segment.h"

idt_callback callback[256];

static uint16_t idtr[5];

void handle_int(struct regs *rs) {
  __asm__ volatile("cli;");
  if (callback[rs->n] != NULL) {
    callback[rs->n](rs);
    apic_send_eoi();
  } else {
    __asm__ volatile("hlt;");
  }
  __asm__ volatile("sti");
}

struct idt_entity {
  uint32_t entry[4];
} __attribute__((aligned(8))) idt_def[256];

extern idt_callback idt_vectors[256];

static void set_gate(idt_callback gate, int vector, uint8_t dpl, int trap, uint8_t ist) {
  uint64_t vaddr = (uint64_t)gate;
  uint32_t type = trap ? 0xF : 0xE;
  idt_def[vector].entry[0] = (vaddr & 0xFFFF) | (KERNEL_CS << 16);
  idt_def[vector].entry[1] = (vaddr & 0xFFFF0000) | (type << 8) | ((dpl & 0x3) << 13) | (1 << 15) | ist;
  idt_def[vector].entry[2] = vaddr >> 32;
  idt_def[vector].entry[3] = 0;
}

void idt_init() {
  for (int i = 0; i < 256; i++) {
    uint8_t ist;
    switch (i) {
    case 8:
      ist = 1;
      break;
    case 2:
      ist = 2;
      break;
    case 1:
    case 3:
      ist = 3;
      break;
    case 18:
      ist = 4;
      break;
    default:
      ist = 5;
      break;
    };
    set_gate(idt_vectors[i], i, 0, 0, ist);
  }
  uint64_t idt_addr = (uint64_t)idt_def;
  idtr[0] = 8*256-1;
  idtr[1] = idt_addr & 0xffff;
  idtr[2] = (idt_addr >> 16) & 0xffff;
  idtr[3] = (idt_addr >> 32) & 0xffff;
  idtr[4] = (idt_addr >> 48) & 0xffff;
}

void idt_init_for_each_proc() {
  __asm__ volatile ("lidt (%0)"::"r"(idtr));
  __asm__ volatile ("sti;");
}

// vector nの割り込み/例外が発生した時に、funcを実行するようにする。
void idt_register_callback(int n, idt_callback func) {
  callback[n] = func;
}
