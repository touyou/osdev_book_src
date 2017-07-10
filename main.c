/*
 *            main.c
 * written by Shinichi Awamoto, 2017
 *
 * 各種ハードウェア初期化関数の呼び出し
 */

#include "acpi.h"
#include "apic.h"
#include "common.h"
#include "framebuffer.h"
#include "idt.h"
#include "multiboot2.h"
#include "segment.h"

void func(struct regs *rs) {
  __asm__ volatile("cli;hlt;");
}

void ipi_handler(struct regs *rs) {}

uint64_t get_nanosec() {
   struct hpet_descriptor *hp = acpi_get_hpet_desc();
   uint64_t addr = hp->address;
   *((uint64_t *)(addr+0x10)) |= 1;
   uint32_t ccp = *((uint32_t *)(addr + 4));
   uint64_t mcv = *((uint64_t *)(addr + 0xf0));
   return ccp * mcv / 1000000;
 }

 void wait(int sec) {
   uint64_t start = get_nanosec();
   while (1) {
     uint64_t cur = get_nanosec();
     if (cur - start > 1000000000) break;
   }
   return;
 }

void cmain() {
  struct rsdp_descriptor *rsdp = multiboot_get_rsdp_desc();
  if (rsdp == NULL) {
    panic();
  }
  acpi_init(rsdp);

  struct multiboot_tag_framebuffer_common *framebuffer = multiboot_get_framebuffer();
  if (framebuffer == NULL) {
    panic();
  }
  framebuffer_init(framebuffer);

  struct apic_descriptor *madt = acpi_get_apic_desc();
  if (madt == NULL) {
    panic();
  }
  apic_init(madt);

  gdt_init();

  idt_init();

  idt_init_for_each_proc();

  apic_enable_lapic();

  apic_start_other_processors();

  // TODO ここにコードを追加
  uint32_t addr = get_baseaddr();
  uint32_t apid = *((uint32_t *)(addr+3));

  framebuffer_printf("%d\n", apid);

  // wait 1s
  wait(1);

  idt_register_callback(32+(int)apid, ipi_handler);
  apic_send_ipi((uint8_t)apid + 1, 32+(int)apid);

  while(1) {
    __asm__ volatile("hlt;");
  }
}

void cmain_for_ap() {
  apic_initialize_ap();

  gdt_init();

  idt_init_for_each_proc();

  __asm__ volatile("hlt;");

  // TODO ここにコードを追加
  uint32_t addr = get_baseaddr();
  uint32_t apid = *((uint32_t *)(addr+3));

  framebuffer_printf("%d\n", apid);

  // wait 1s
  wait(1);

  idt_register_callback(32+(int)apid, ipi_handler);
  apic_send_ipi((uint8_t)apid + 1, 32+(int)apid);

  while(1) {
    __asm__ volatile("hlt;");
  }
}
