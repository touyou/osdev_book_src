/*
 *            main.c
 * written by Shinichi Awamoto, 2017
 * 
 * 各種ハードウェア初期化関数の呼び出し
 */

#include "common.h"
#include "multiboot2.h"
#include "acpi.h"
#include "framebuffer.h"

uint32_t multiboot_info;

uint64_t get_nanosec() {
  struct hpet_descriptor *hp = acpi_get_hpet_desc();
  uint64_t addr = hp->address;
  *((uint64_t *)(addr+0x10)) |= 1;
  uint32_t ccp = *((uint32_t *)(addr + 4));
  uint64_t mcv = *((uint64_t *)(addr + 0xf0));
  // 三回MMIOを行っているので3で割っている
  return ccp * mcv / 1000000 / 3;
}

void cost_exec() {
  // HPET 取得コスト
  uint64_t sec_arr[101];
  uint64_t sum = 0;
  for(int i=0; i<101; ++i) {
    sec_arr[i] = get_nanosec();
    if (i != 0) sum += sec_arr[i] - sec_arr[i-1];
  }
  uint64_t d_sum = 0;
  for(int i=1; i<101; ++i) {
    d_sum += (sec_arr[i]-sec_arr[i-1]-sum/100)*(sec_arr[i]-sec_arr[i-1]-sum/100);
  }
  framebuffer_printf("ave. %d disp. %d", sum / 100, d_sum / 100);

  // DMAコスト
  struct hpet_descriptor *hp = acpi_get_hpet_desc();
  uint64_t addr = hp->address;
  *((uint64_t *)(addr+0x10)) |= 1;
  uint32_t ccp = *((uint32_t *)(addr + 4));
  uint64_t res = framebuffer_ntimesput((uint64_t *)(addr + 0xf0));
  framebuffer_printf(" dma. %d nsec\n", ccp*res/1000000);
}

void cmain() {
  // TODO check multiboot2 magic number

  // multiboot_info について
  // multiboot_info にはentryの直後でebxレジスタが代入されている。
  // この値は本来32bit物理アドレスだが、現在は1GBをストレートマップ
  // しているため、1GB以下の空間については物理＝リニア＝論理とみなせる。
  // かつ、64bitのうちの上位32bitについては0で埋めたとみなせば、
  // 期待通りアクセスできる。
  // よって、uint32_tをポインタ(64bit)にキャストしても正常に動作する。

  // コードはmultiboot specification 2 仕様書より
  for (struct multiboot_tag *tag = (struct multiboot_tag *)(uint64_t)(multiboot_info + 8); tag->type != MULTIBOOT_TAG_TYPE_END; tag = (struct multiboot_tag *) ((multiboot_uint8_t *) tag + ((tag->size + 0b111) & ~0b111))) {
    switch(tag->type) {
    case MULTIBOOT_TAG_TYPE_ACPI_OLD:
      {
        // ACPI RSDPテーブルの取得
        struct multiboot_tag_old_acpi *acpi = (struct multiboot_tag_old_acpi *)tag;
        acpi_init((struct rsdp_descriptor *)acpi->rsdp);
      }
      break;
    case MULTIBOOT_TAG_TYPE_FRAMEBUFFER:
      {
        // framebuffer tagの取得
        framebuffer_init((struct multiboot_tag_framebuffer_common *)tag);
      }
      break;
    default:
      break;
    }
  }
  
  // TODO ここにコードを追加
  for (int i=0; i<10; i++) cost_exec();
  
  while(1) {
    __asm__ volatile("hlt;");
  }
}

void panic() {
  while(1) {
    __asm__ volatile("cli;hlt;nop;");
  }
}
