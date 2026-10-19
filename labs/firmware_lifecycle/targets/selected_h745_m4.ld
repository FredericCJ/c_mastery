ENTRY(st_reset)
MEMORY { CODE (rx) : ORIGIN = 0x08100000, LENGTH = 128K
 RAM (rwx) : ORIGIN = 0x30000000, LENGTH = 128K }
SECTIONS {
 .vectors : { KEEP(*(.vectors)) } >CODE
 .text : { *(.text*) *(.rodata*) } >CODE
 .data : { . = ALIGN(4); _data_start = .; *(.data*) . = ALIGN(4); _data_end = .; } >RAM AT>CODE
 _data_load = LOADADDR(.data);
 .bss (NOLOAD) : { . = ALIGN(4); _bss_start = .; *(.bss*) *(COMMON) . = ALIGN(4); _bss_end = .; } >RAM
 _stack_top = ORIGIN(RAM)+LENGTH(RAM);
 ASSERT(LOADADDR(.data)+SIZEOF(.data)<=ORIGIN(CODE)+LENGTH(CODE),"bootstrap crosses owned sector")
 ASSERT(_bss_end<ORIGIN(RAM)+0x10000,"reserve bounded64KiB stack")
 ASSERT(ADDR(.vectors)%1024==0,"target vector alignment")
 /DISCARD/ : { *(.comment) *(.ARM.exidx*) *(.ARM.extab*) }
}
