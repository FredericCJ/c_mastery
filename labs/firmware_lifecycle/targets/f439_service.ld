ENTRY(st_reset)
MEMORY { CODE (rx) : ORIGIN = 0x8120000, LENGTH = 256K
 RAM (rwx) : ORIGIN = 0x20000000, LENGTH = 64K }
SECTIONS {
 .vectors : { KEEP(*(.vectors)) } >CODE
  .app_report 0x8120400 : { KEEP(*(.app_report)) } >CODE
 .text : { *(.text*) *(.rodata*) } >CODE
 .data : { . = ALIGN(4); _data_start = .; *(.data*) . = ALIGN(4); _data_end = .; } >RAM AT>CODE
 _data_load = LOADADDR(.data);
 .bss (NOLOAD) : { . = ALIGN(4); _bss_start = .; *(.bss*) *(COMMON) . = ALIGN(4); _bss_end = .; } >RAM
 _stack_top = ORIGIN(RAM)+LENGTH(RAM);
 ASSERT(LOADADDR(.data)+SIZEOF(.data)<=ORIGIN(CODE)+LENGTH(CODE),"F439 service crosses owned bank2 region")
 ASSERT(_bss_end<ORIGIN(RAM)+LENGTH(RAM)-0x4000,"reserve bounded16KiB service stack")
 ASSERT(ADDR(.vectors)%1024==0,"target vector alignment")
 /DISCARD/ : { *(.comment) *(.ARM.exidx*) *(.ARM.extab*) }
}
