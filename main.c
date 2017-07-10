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
#include "paging.h"

int status = 0;

extern uint64_t pt1[256];

void fault_handler(struct regs *rs) {
    pt1[0] = PTE_PRESENT_BIT | PTE_WRITE_BIT | PTE_USER_BIT | (0x1000000 + 4096);
}

void ipi_handler(struct regs *rs) {
}

void ipi_recv_handler(struct regs *rs) {
    uint64_t m = 0x1000100;
    __asm__ volatile("invlpg (%0)"::"b"(m):"memory");
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

  // TODO 課題A: ここでページフォルトハンドラを登録する事
  idt_register_callback(14, fault_handler);

  int *ptr = (int *)((uint64_t)0x4000000000);

  *ptr = 0;
  framebuffer_printf("[%d:%d]", apic_get_id(), *ptr);

  __sync_fetch_and_add(&status, 1);

  apic_start_other_processors();

  while(status != apic_get_cpu_nums()) {
  }

  pt1[0] = PTE_PRESENT_BIT | PTE_WRITE_BIT | PTE_USER_BIT | (0x1000000 + 4096);
  *ptr = 0;
  framebuffer_printf("<%d:%d>", apic_get_id(), *ptr);

  // TODO 課題B: TLB shootdown通知コードをここに書く事
  for (int i = 1; i < apic_get_cpu_nums; i++) {
      idt_register_callback(32+apic_get_id(), ipi_handler);
      apic_send_ipi(apic_get_id()+i, 32+apic_get_id());
  }

  __sync_fetch_and_add(&status, 1);

  while(1) {
    __asm__ volatile("hlt;");
  }
}

void cmain_for_ap() {
  apic_initialize_ap();

  gdt_init();

  idt_init_for_each_proc();

  // TODO 課題B: ここでTLB shootdown受信ハンドラを登録する事
  idt_register_callback(32+apic_get_id(), ipi_recv_handler);

  int *ptr = (int *)((uint64_t)0x4000000000);

  while(status != apic_get_id()) {
  }

  (*ptr)++;
  framebuffer_printf("[%d:%d]", apic_get_id(), *ptr);

  __sync_fetch_and_add(&status, 1);

  while(status != apic_get_id() + apic_get_cpu_nums()) {
  }

  (*ptr)++;
  framebuffer_printf("<%d:%d>", apic_get_id(), *ptr);

  __sync_fetch_and_add(&status, 1);

  while(1) {
    __asm__ volatile("hlt;");
  }
}
