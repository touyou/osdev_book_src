/*
 *            acpi.h
 * written by Shinichi Awamoto, 2017
 * 
 * ACPI関連の関数の宣言、構造体の定義
 */

#pragma once

#include "common.h"

struct rsdp_descriptor {
  char signature[8];
  uint8_t checksum;
  char oemid[6];
  uint8_t revision;
  uint32_t rsdt_address;
} __attribute__((packed));

struct acpi_sdt_header {
  char signature[4];
  uint32_t length;
  uint8_t revision;
  uint8_t checksum;
  char oemid[6];
  char oem_table_id[8];
  uint32_t oem_revision;
  uint32_t creator_id;
  uint32_t creator_revison;
} __attribute__((packed));

struct hpet_descriptor {
  struct acpi_sdt_header header;
  uint32_t event_timer_block_id;
  uint8_t address_space_id;
  uint8_t register_bit_width;
  uint8_t register_bit_offset;
  uint8_t reserved;
  uint64_t address;
  uint8_t hpet_number;
  uint16_t minimum_tick;
  uint8_t page_protection;
} __attribute__((packed));

void acpi_init(struct rsdp_descriptor *rsdp);
struct hpet_descriptor *acpi_get_hpet_desc();
