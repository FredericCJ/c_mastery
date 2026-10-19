ENTRY(_start)
SECTIONS { . = 0x40080000; .text : { *(.text.start) *(.text*) } .rodata : { *(.rodata*) } .data : { *(.data*) } __bss_start = .; .bss : { *(.bss*) *(COMMON) } __bss_end = .; }
