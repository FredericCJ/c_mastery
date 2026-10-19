ENTRY(reset)
MEMORY { FLASH (rx) : ORIGIN = 0, LENGTH = 4M
         RAM (rwx) : ORIGIN = 0x20000000, LENGTH = 4M }
__stack_top = ORIGIN(RAM) + LENGTH(RAM);
SECTIONS {
 .text : { KEEP(*(.vectors)) *(.text*) *(.rodata*) } > FLASH
 .data : { *(.data*) } > RAM AT> FLASH
 .bss (NOLOAD) : { *(.bss*) *(COMMON) } > RAM
 ASSERT(SIZEOF(.data) == 0, "this bounded ABI specimen has no initialized RAM")
 ASSERT(SIZEOF(.bss) == 0, "this bounded ABI specimen has no zero-fill RAM")
 ASSERT((__stack_top & 7) == 0, "eight-byte public-interface stack")
}
