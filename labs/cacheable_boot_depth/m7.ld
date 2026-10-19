ENTRY(cb_reset)
MEMORY { CODE (rx) : ORIGIN  =  0x08000000, LENGTH  =  128K
 RAM (rwx) : ORIGIN  =  0x20000000, LENGTH  =  128K
 PEER (rw) : ORIGIN  =  0x30040000, LENGTH  =  32 }
SECTIONS {
 .vectors : { _vectors =  .; KEEP(*(.vectors)) } >CODE
 .text : { *(.text*) *(.rodata*) } >CODE
 .data : { .  =  ALIGN(4); _data_start =  .; *(.data*) .  =  ALIGN(4); _data_end =  .; } >RAM AT>CODE
 _data_load = LOADADDR(.data);
 .bss(NOLOAD) : { .  =  ALIGN(4); _bss_start =  .; *(.bss*) *(COMMON) .  =  ALIGN(4); _bss_end =  .; } >RAM
 .peer(NOLOAD) : { KEEP(*(.peer)) } >PEER
 _stack_top = ORIGIN(RAM)+LENGTH(RAM);
 ASSERT(SIZEOF(.peer)==32,"exact one owned peer line")
 ASSERT(ADDR(.peer)==0x30040000,"fixed peer placement")
 ASSERT(_bss_end<=_stack_top-4096,"private4KiB stack reserve")
 /DISCARD/ : { *(.ARM.exidx*) *(.ARM.extab*) *(.comment) }
}
