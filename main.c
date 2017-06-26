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
#include "multiboot2.h"

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

  apic_enable_lapic();

  apic_start_other_processors();
  
  // TODO ここにコードを追加

  while(1) {
    __asm__ volatile("hlt;");
  }
}

void cmain_for_ap() {
  apic_initialize_ap();

  // TODO ここにコードを追加

  while(1) {
    __asm__ volatile("hlt;");
  }
}
