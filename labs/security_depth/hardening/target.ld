ENTRY(sh_entry)
MEMORY { CODE (rx) : ORIGIN = 0x00000000, LENGTH = 512K
         RAM (rw) : ORIGIN = 0x20000000, LENGTH = 128K }
SECTIONS {
 .text : { *(.text*) *(.rodata*) } > CODE
 .data : { *(.data*) } > RAM AT> CODE
 .bss (NOLOAD) : { *(.bss*) *(COMMON) } > RAM
 /DISCARD/ : { *(.comment) *(.note*) }
}
