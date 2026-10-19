ENTRY(Reset)
MEMORY { CODE (rx) : ORIGIN = 0x00000000, LENGTH = 1M
         RAM (rwx) : ORIGIN = 0x20000000, LENGTH = 128K }
SECTIONS {
 .vectors : { KEEP(*(.vectors)) } > CODE
 .text : { *(.text*) *(.rodata*) } > CODE
 .data : { . = ALIGN(4); _data_start = .; *(.data*) . = ALIGN(4); _data_end = .; } > RAM AT>CODE
 _data_load = LOADADDR(.data);
 .bss (NOLOAD) : { . = ALIGN(4); _bss_start = .; *(.bss*) *(COMMON) . = ALIGN(4); _bss_end = .; } > RAM
 .candidate 0x00040000 : { KEEP(*(.candidate)) } > CODE
 _stack_top = 0x20020000;
 _app_stack = 0x2001F000;
 _app_data = 0x2001E000;
 _app_bss = 0x2001E004;
 ASSERT(_bss_end < 0x20010000, "boot data must leave bounded boot/app stacks")
 ASSERT(SIZEOF(.candidate) <= 2048, "bounded signed candidate image")
 ASSERT(ADDR(.candidate) % 512 == 0, "VTOR alignment")
 /DISCARD/ : { *(.comment) *(.ARM.exidx*) *(.ARM.extab*) }
}
