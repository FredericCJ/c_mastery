ENTRY(_start)
SECTIONS {
 . = 0x100000;
 .multiboot : { KEEP(*(.multiboot)) }
 .text ALIGN(4096) : { *(.text.boot) *(.text*) }
 .rodata ALIGN(4096) : { *(.rodata*) }
 .data ALIGN(4096) : { *(.data*) }
 .bss ALIGN(4096) : { *(COMMON) *(.bss*) }
 kernel_end = .;
 /DISCARD/ : { *(.eh_frame*) *(.comment*) *(.note*) }
}
