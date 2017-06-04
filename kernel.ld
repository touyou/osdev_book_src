OUTPUT_FORMAT(elf64-x86-64)

/* ブートローダが呼び出すラベル */
ENTRY (entry)

SECTIONS{
  /* multibootの都合上、各セクションは0x100000以降に置かれなければならない */
  . = 0x100000;
  .boot : {
    *(.bootstrap*) 
  }

  . = LOADADDR(.boot) + SIZEOF(.boot);
  .text : {
    *(.text*)
    *(.gnu.linkonce.t*)
  }

  .rodata : {
    *(.rodata*)
    *(.gnu.linkonce.r*)
  }

  .data : {
    *(.data*)
    *(.gnu.linkonce.d*)
  }

  .bss : {
    *(COMMON*)
    *(.COMMON*)
    *(.bss*)
    *(.gnu.linkonce.b*)
  }
}
