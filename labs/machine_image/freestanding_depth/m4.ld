ENTRY(fr_reset)
MEMORY { CODE(rx): ORIGIN = 0, LENGTH = 4M
         RAM(rw): ORIGIN = 0x20000000, LENGTH = 4M }
__stack_top = ORIGIN(RAM) + LENGTH(RAM);
SECTIONS {
 .text : { KEEP(*(.vectors)) *(.text*) *(.rodata*) . = ALIGN(8); } > CODE
 .ARM.exidx : { *(.ARM.exidx*) } > CODE
 .data : { . = ALIGN(8); __data_start = .; *(.data*) . = ALIGN(8); __data_end = .; } > RAM AT> CODE
 __data_load = LOADADDR(.data);
 .bss (NOLOAD) : { . = ALIGN(8); __bss_start = .; *(.bss*) *(COMMON) . = ALIGN(8); __bss_end = .; } > RAM
 ASSERT(__bss_end + 65536 <= __stack_top,"static storage overlaps reserved stack")
}
