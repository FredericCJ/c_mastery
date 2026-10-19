ENTRY(cl_reset)
MEMORY {
  FLASH (rx) : ORIGIN = 0, LENGTH = 512K
  RAM (rwx) : ORIGIN = 0x20000000, LENGTH = 64K
}
cl_stack_top = ORIGIN(RAM) + LENGTH(RAM);
SECTIONS {
  .text : { KEEP(*(.vectors)) *(.text*) *(.rodata*) . = ALIGN(4); } > FLASH
  .data : { cl_data_start = .; *(.data*) . = ALIGN(4); cl_data_end = .; } > RAM AT> FLASH
  cl_data_load = LOADADDR(.data);
  .bss (NOLOAD) : { . = ALIGN(4); cl_bss_start = .; *(.bss*) *(COMMON) . = ALIGN(4); cl_bss_end = .; } > RAM
  /DISCARD/ : { *(.ARM.exidx*) *(.ARM.extab*) *(.comment*) }
  ASSERT(cl_bss_end + 8192 < cl_stack_top, "stack reserve")
}
