ENTRY(reset)
MEMORY { FLASH (rx) : ORIGIN = 0, LENGTH = 256K
 RAM (rwx) : ORIGIN = 0x20000000, LENGTH = 16K }
SECTIONS {
 .vectors : { KEEP(*(.vectors)) } > FLASH
 .text : { *(.text*) *(.rodata*) } > FLASH
 .data : { *(.data*) } > RAM AT> FLASH
 .bss (NOLOAD) : { *(.bss*) *(COMMON) . = ALIGN(8); test_guard = .; . += 16; test_low = .; . += 128; test_top = .; } > RAM
 stack_top = ORIGIN(RAM)+LENGTH(RAM);
 ASSERT(test_top < stack_top-1024,"test stack and caller stack must be distinct")
}
