OUTPUT_FORMAT(elf64-x86-64)

/* ブートローダが呼び出すラベル */
ENTRY (entry)

SECTIONS{
  kLinearAddrOffset = 0xffffffff80000000;

  /* multibootの都合上、各セクションは0x100000以降に置かれなければならない */
  . = 0x100000;
  .boot : {
    *(.bootstrap*) 
    boot.o (.text*)
    boot.o (.rodata*)
    boot.o (.data*)
    boot.o (.bss*)
  }

  . = kLinearAddrOffset + LOADADDR(.boot) + SIZEOF(.boot);
  .text : AT(LOADADDR(.boot) + SIZEOF(.boot)) {
    *(EXCLUDE_FILE (boot.o) .text*)
    *(.gnu.linkonce.t*)
  }

  .rodata : {
    *(EXCLUDE_FILE (boot.o) .rodata*)
    *(.gnu.linkonce.r*)
  }

  .data : {
    *(EXCLUDE_FILE (boot.o) .data*)
    *(.gnu.linkonce.d*)
  }

  .bss : {
    *(COMMON*)
    *(.COMMON*)
    *(EXCLUDE_FILE (boot.o) .bss*)
    *(.gnu.linkonce.b*)
  }
}
