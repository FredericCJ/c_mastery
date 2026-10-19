/* The same word-aligned reset-copy intervals as the original sort harness. */
ENTRY(reset)
MEMORY { FLASH (rx) : ORIGIN = 0x00000000, LENGTH = 4M
         RAM (rwx) : ORIGIN = 0x20000000, LENGTH = 4M }
__stack_top = ORIGIN(RAM) + LENGTH(RAM);
SECTIONS {
 .text : { KEEP(*(.vectors)) *(.text*) *(.rodata*) . = ALIGN(4); } > FLASH
 .data : ALIGN(4) { __data_start = .; *(.data*) . = ALIGN(4); __data_end = .; } > RAM AT> FLASH
 __data_load = LOADADDR(.data);
 .bss (NOLOAD) : ALIGN(4) { __bss_start = .; *(.bss*) *(COMMON) . = ALIGN(4); __bss_end = .; } > RAM
 ASSERT(((__data_load | __data_start | __data_end | __bss_start | __bss_end) & 3) == 0, "startup word intervals must be aligned")
 ASSERT(__bss_end + 32768 < __stack_top, "bounded stack reserve")
}
