ENTRY(_start)
SECTIONS {
 . = 0x100000;
 .rodata : { *(.rodata*) }
 .multiboot ALIGN(8) : { KEEP(*(.multiboot)) }
 .text ALIGN(4096) : { *(.text.boot) *(.text*) }

 .data ALIGN(4096) : { *(.data*) }
 .bss ALIGN(4096) : { *(COMMON) *(.bss*) }
 kernel_end = .;
 /DISCARD/ : { *(.eh_frame*) *(.comment*) *(.note*) }
}
