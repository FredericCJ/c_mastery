ENTRY(af_reset)
MEMORY { FLASH (rx) : ORIGIN = 0x8000000, LENGTH = 1M
 RAM (rwx) : ORIGIN = 0x20000000, LENGTH = 128K
 SHARED (rw) : ORIGIN = 0x38000000, LENGTH = 8K }
__stack_top = ORIGIN(RAM)+LENGTH(RAM);
SECTIONS {
 .text : { KEEP(*(.vectors)) *(.text*) *(.rodata*) . = ALIGN(4); } > FLASH
 .data : { __data_start = .; *(.data*) . = ALIGN(4); __data_end = .; } > RAM AT>FLASH
 __data_load = LOADADDR(.data);
 .bss(NOLOAD) : { . = ALIGN(8); __bss_start = .; *(.bss*) *(COMMON) . = ALIGN(8); __bss_end = .; } > RAM
 .shared(NOLOAD) : { __shared_start = .; . += 8192; __shared_end = .; } > SHARED
 /DISCARD/ : { *(.ARM.exidx*) *(.ARM.extab*) *(.comment*) }
 ASSERT(__bss_end+8192<__stack_top,"private storage plus stack")
}
