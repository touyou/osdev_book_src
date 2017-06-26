/*
 *            acpi.h
 * written by Shinichi Awamoto, 2017
 * 
 * ACPI関連の関数の定義
 */

#include "acpi.h"
#include "common.h"

struct rsdt {
  struct acpi_sdt_header h;
  uint32_t ptr_to_other_sdt[0];
} __attribute__((packed));

static struct rsdt *rsdt = NULL;

void acpi_init(struct rsdp_descriptor *rsdp) {
  if (rsdp == NULL) {
    panic();
  }
  if (__builtin_strncmp(rsdp->signature, "RSD PTR ", 8) == 0) {
    rsdt = (struct rsdt *)(uint64_t)rsdp->rsdt_address;
  }
}

// ACPI 2.0 HPET Description Tableを取得するための関数
// 戻り値：見つからなかった場合はNULLを返す
struct hpet_descriptor *acpi_get_hpet_desc() {
  if (rsdt == NULL) {
    return NULL;
  }

  int entries = (rsdt->h.length - sizeof(rsdt->h)) / 4;

  for (int i = 0; i < entries; i++) {
    struct acpi_sdt_header *h = (struct acpi_sdt_header *)(uint64_t)rsdt->ptr_to_other_sdt[i];
    if (__builtin_strncmp(h->signature, "HPET", 4) == 0) {
      struct hpet_descriptor *hpet = (struct hpet_descriptor *)h;
      return hpet;
    }
  }

  return NULL;
}

// MADT取得関数
// 戻り値：見つからなかった場合はNULLを返す
struct apic_descriptor *acpi_get_apic_desc() {
  if (rsdt == NULL) {
    return NULL;
  }

  int entries = (rsdt->h.length - sizeof(rsdt->h)) / 4;

  for (int i = 0; i < entries; i++) {
    struct acpi_sdt_header *h = (struct acpi_sdt_header *)(uint64_t)rsdt->ptr_to_other_sdt[i];
    if (__builtin_strncmp(h->signature, "APIC", 4) == 0) {
      struct apic_descriptor *madt = (struct apic_descriptor *)h;
      return madt;
    }
  }

  return NULL;
}
