ENTRY(reset_handler)
MEMORY { FLASH (rx) : ORIGIN = 0, LENGTH = 256K
 RAM (rwx) : ORIGIN = 0x20000000, LENGTH = 64K }
SECTIONS { .vectors : { KEEP(*(.vectors)) } > FLASH
 .text : { *(.text*) *(.rodata*) } > FLASH
 .data : { *(.data*) } > RAM AT> FLASH
 .bss (NOLOAD) : { *(.bss*) *(COMMON) } > RAM
 /DISCARD/ : { *(.ARM.exidx*) *(.comment*) } }
