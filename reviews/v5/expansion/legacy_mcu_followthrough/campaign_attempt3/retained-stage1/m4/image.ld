ENTRY(cmx_reset)
MEMORY { FLASH (rx) : ORIGIN = 0x0, LENGTH = 1024K
RAM (rwx) : ORIGIN = 0x20000000, LENGTH = 128K }
__stack_top = ORIGIN(RAM) + LENGTH(RAM);
SECTIONS {
 .text : { KEEP(*(.vectors)) *(.text*) *(.rodata*) . = ALIGN(4); } > FLASH
 .data : { . = ALIGN(4); __data_start = .; *(.data*) . = ALIGN(4); __data_end = .; } > RAM AT>FLASH
 __data_load = LOADADDR(.data);
 .bss (NOLOAD) : { . = ALIGN(8); __bss_start = .; *(.bss*) *(COMMON) . = ALIGN(8); __bss_end = .; } > RAM
 /DISCARD/ : { *(.ARM.exidx*) *(.ARM.extab*) *(.comment*) }
 ASSERT(__bss_end+8192<__stack_top,"static storage plus handler stack exceeds private RAM")
}
