ENTRY(xf_reset)
MEMORY { FLASH (rx) : ORIGIN = 0,LENGTH = 256K
RAM (rwx) : ORIGIN = 0x20000000,LENGTH = 64K }
__stack_top = ORIGIN(RAM)+LENGTH(RAM);
SECTIONS {
 .vectors : ALIGN(1024) { KEEP(*(.vectors)) } >FLASH
 .text : { *(.text*) *(.rodata*) . = ALIGN(4); } >FLASH
 .data : ALIGN(4) { __data_start = .; *(.data*) . = ALIGN(4); __data_end = .; } >RAM AT>FLASH
 __data_load = LOADADDR(.data);
 .bss (NOLOAD) : ALIGN(8) { __bss_start = .; *(.bss*) *(COMMON) . = ALIGN(4); __bss_end = .; } >RAM
 ASSERT(__bss_end+16384<__stack_top,"wire test stack reserve")
 ASSERT(SIZEOF(.vectors)==1024,"complete vector table")
 /DISCARD/ : { *(.ARM.exidx*) *(.ARM.extab*) *(.comment) }
}
