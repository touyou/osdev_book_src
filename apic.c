/*
 *            acpi.h
 * written by Shinichi Awamoto, 2017
 * 
 * APIC関連の関数の定義
 */

#include "apic.h"
#include "acpi.h"
#include "common.h"

struct apic_manager {
  int ncpu;
  uint8_t lapicid[MAX_CPU];
  uint32_t lapic_base_addr;
};

static struct apic_manager manager;


#define LAPIC_REG_OFFSET_EOI (0xB)
#define LAPIC_REG_OFFSET_SVR (0xF)
#define LAPIC_REG_OFFSET_LVT_CMCI (0x2f)
#define LAPIC_REG_OFFSET_ICR_LOW (0x30)
#define LAPIC_REG_OFFSET_ICR_HIGH (0x31)
#define LAPIC_REG_OFFSET_LVT_TIMER (0x32)
#define LAPIC_REG_OFFSET_LVT_THERMAL_SENSOR (0x33)
#define LAPIC_REG_OFFSET_LVT_PERFORMANCE_COUNTER (0x34)
#define LAPIC_REG_OFFSET_LVT_LINT0 (0x35)
#define LAPIC_REG_OFFSET_LVT_LINT1 (0x36)
#define LAPIC_REG_OFFSET_LVT_ERROR (0x37)

// Intel SDM Vol 3, vol3 Figure 10-8 (Local Vector Table)
#define LAPIC_REG_LVT_MASK (1 << 16)

// Intel SDM Vol 3, Figure 10-12 (Interrupt Command Register) より
#define LAPIC_REG_ICR_LEVEL_ASSERT (1 << 14)
#define LAPIC_REG_ICR_TRIGGER_MODE_EDGE (0 << 15)
#define LAPIC_REG_ICR_TRIGGER_MODE_LEVEL (1 << 15)
#define LAPIC_REG_ICR_DESTSHORTHAND_NOSHORTHAND (0 << 18)

// Intel SDM Vol 3, Figure 10-23. Spurious-Interrupt Vector Register (SVR)より
#define LAPIC_REG_SVR_FLAG_APIC_ENABLE (1 << 8)

// Intel SDM Vol 3, vol3 10.5.1 (Delivery Mode) より
#define LAPIC_DELIVERY_MODE_FIXED (0 << 8)
#define LAPIC_DELIVERY_MODE_INIT (5 << 8)
#define LAPIC_DELIVERY_MODE_STARTUP (6 << 8)

static inline void write_lapic_reg(int offset, uint32_t data) {
  *(uint32_t *)((uint64_t)manager.lapic_base_addr + offset * 0x10) = data;
}

static inline uint32_t read_lapic_reg(int offset) {
  return *(uint32_t *)((uint64_t)manager.lapic_base_addr + offset * 0x10);
}

static inline void write_icr(uint8_t dest, uint32_t flags) {
  write_lapic_reg(LAPIC_REG_OFFSET_ICR_HIGH, dest << 24);
  read_lapic_reg(0x2); // 書き込みを待つ
  write_lapic_reg(LAPIC_REG_OFFSET_ICR_LOW, flags);
  write_lapic_reg(LAPIC_REG_OFFSET_ICR_LOW, read_lapic_reg(LAPIC_REG_OFFSET_ICR_LOW) | ((flags & LAPIC_DELIVERY_MODE_INIT) ? 0 : LAPIC_REG_ICR_LEVEL_ASSERT));
  read_lapic_reg(0x2); // 書き込みを待つ
}

// APICの初期化
void apic_init(struct apic_descriptor *madt) {
  manager.lapic_base_addr = madt->local_interrupt_controller_address;
  manager.ncpu = 0;
  
  // MADTを解析して、Local APIC IDの一覧を取得する
  for (uint32_t offset = sizeof(struct apic_descriptor); offset < madt->header.length;) {
    struct apic_struct_header *header = (struct apic_struct_header *)((uint8_t *)madt + offset);
    switch(header->type) {
    case APIC_STRUCT_TYPE_LAPIC: {
      struct local_apic_struct *lapic = (struct local_apic_struct *)header;
      if ((lapic->flags & LOCAL_APIC_STRUCT_FLAG_ENABLE) == 0) {
        break;
      }
      manager.lapicid[manager.ncpu] = lapic->lapic_id;
      manager.ncpu++;
      break;
    }
    default: {
      break;
    }
    }
    if (manager.ncpu == MAX_CPU) {
      break;
    }
    offset += header->length;
  }
}

// Local APICの初期化
// かなり手抜き
// 正しい手順はIntel SDM Vol3, 10.4.3 Enabling or Disabling the Local APIC を参照する事
void apic_enable_lapic() {
  write_lapic_reg(LAPIC_REG_OFFSET_LVT_TIMER, LAPIC_REG_LVT_MASK);
  write_lapic_reg(LAPIC_REG_OFFSET_LVT_CMCI, LAPIC_REG_LVT_MASK);
  write_lapic_reg(LAPIC_REG_OFFSET_LVT_THERMAL_SENSOR, LAPIC_REG_LVT_MASK);
  write_lapic_reg(LAPIC_REG_OFFSET_LVT_PERFORMANCE_COUNTER, LAPIC_REG_LVT_MASK);
  write_lapic_reg(LAPIC_REG_OFFSET_LVT_LINT0, LAPIC_REG_LVT_MASK);
  write_lapic_reg(LAPIC_REG_OFFSET_LVT_LINT1, LAPIC_REG_LVT_MASK);
  write_lapic_reg(LAPIC_REG_OFFSET_LVT_ERROR, LAPIC_DELIVERY_MODE_FIXED);
  write_lapic_reg(LAPIC_REG_OFFSET_SVR, LAPIC_REG_SVR_FLAG_APIC_ENABLE);
}

void apic_send_eoi() {
  write_lapic_reg(LAPIC_REG_OFFSET_EOI, 0);
}

// destidで指定したAPIC IDに対してIPIを送信する
// IPIを受け取ったコアは、vector番の割り込みハンドラを実行する
void apic_send_ipi(uint8_t destid, int vector) {
  write_icr(destid, LAPIC_DELIVERY_MODE_FIXED | LAPIC_REG_ICR_TRIGGER_MODE_LEVEL | LAPIC_REG_ICR_DESTSHORTHAND_NOSHORTHAND | vector);
}

void entryothers(); 

#define IO_PORT_RTC (0x70)

// APを起動
static void start_ap(uint8_t apicid) {
  // 参照：mp spec Appendix B.5
  outb(IO_PORT_RTC, 0xf); // shutdown code
  outb(IO_PORT_RTC + 1, 0xa);
  uint16_t *warm_reset_vector = (uint16_t *)((0x40 << 4) | 0x67);
  warm_reset_vector[0] = 0;
  warm_reset_vector[1] = ((uint64_t)entryothers >> 4) & 0xffff;

  // universal startup algorithm
  // 参照：mp spec Appendix B.4.1
  write_icr(apicid, LAPIC_DELIVERY_MODE_INIT | LAPIC_REG_ICR_TRIGGER_MODE_LEVEL | LAPIC_REG_ICR_LEVEL_ASSERT);
  // TODO wait 200us
  write_icr(apicid, LAPIC_DELIVERY_MODE_INIT | LAPIC_REG_ICR_TRIGGER_MODE_LEVEL | LAPIC_REG_ICR_LEVEL_ASSERT);
  // TODO wait 100us

  // application processor setup
  // 参照：mp spec Appendix B.4.2
  for (int i = 0; i < 2; i++) {
    write_icr(apicid, LAPIC_DELIVERY_MODE_STARTUP | (((uint64_t)entryothers >> 12) & 0xff));
    // TODO wait 200us
  }
}

extern uint64_t boot16_start;
extern uint64_t boot16_end;

extern uint64_t stack_for_ap[1];

uint8_t stack_buffer_for_ap[(MAX_CPU - 1) * 4096] __attribute__ ((aligned (4096)));

static int is_ap_booted;

// BSP以外のコア(AP)を全て起動するルーチン
void apic_start_other_processors() {
  if (((uint64_t)&boot16_end - (uint64_t)&boot16_start) != 0x1000) {
    panic();
  }
  // bootap.Sの内容を0x8000に移動する
  __builtin_memcpy((uint8_t *)0x8000, &boot16_start, 0x1000);

  if (((uint64_t)stack_buffer_for_ap % 4096) != 0) {
    panic();
  }

  // APを起動
  for (int i = 1; i < manager.ncpu; i++) {
    is_ap_booted = 0;
    stack_for_ap[0] = (uint64_t)&stack_buffer_for_ap[i * 4096];
    
    start_ap(i);
    
    while(!is_ap_booted) {
      __asm__ volatile("pause;":::"memory");
    }
  }
}

// AP用のAPIC初期化ルーチン
void apic_initialize_ap() {
  is_ap_booted = 1;
  
  apic_enable_lapic();
}
