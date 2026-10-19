ENTRY(cb_reset)
MEMORY { CODE (rx) : ORIGIN  =  0, LENGTH  =  512K
 RAM (rwx) : ORIGIN  =  0x20000000, LENGTH  =  128K }
SECTIONS {
 .vectors : { _vectors =  .; KEEP(*(.vectors)) } >CODE
 .text : { *(.text*) *(.rodata*) } >CODE
 .data : { .  =  ALIGN(4); _data_start =  .; *(.data*) .  =  ALIGN(4); _data_end =  .; } >RAM AT>CODE
 _data_load = LOADADDR(.data);
 .bss(NOLOAD) : { .  =  ALIGN(4); _bss_start =  .; *(.bss*) *(COMMON) .  =  ALIGN(4); _bss_end =  .; } >RAM
 _stack_top = ORIGIN(RAM)+LENGTH(RAM);
 ASSERT(_bss_end<=_stack_top-4096,"private stack reserve")
 /DISCARD/ : { *(.ARM.exidx*) *(.ARM.extab*) *(.comment) }
}
