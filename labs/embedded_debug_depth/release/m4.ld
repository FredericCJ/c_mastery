ENTRY(dt_boot)
MEMORY { FLASH (rx) : ORIGIN = 0, LENGTH = 512K
         RAM (rwx) : ORIGIN = 0x20000000, LENGTH = 64K }
dt_stack_top = ORIGIN(RAM) + LENGTH(RAM);
SECTIONS {
 .text : { KEEP(*(.vectors)) *(.text*) *(.rodata*) . = ALIGN(4); } > FLASH
 .data : { dt_data_start = .; *(.data*) . = ALIGN(4); dt_data_end = .; } > RAM AT> FLASH
 dt_data_load = LOADADDR(.data);
 .bss(NOLOAD) : { . = ALIGN(4); dt_bss_start = .; *(.bss*) *(COMMON) . = ALIGN(4); dt_bss_end = .; } > RAM
 .noinit(NOLOAD) : { . = ALIGN(4); *(.noinit*) . = ALIGN(4); } > RAM
 /DISCARD/ : { *(.ARM.exidx*) *(.ARM.extab*) *(.comment*) }
 ASSERT(dt_bss_end+8192<dt_stack_top,"stack reserve")
}
