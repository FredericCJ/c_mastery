ENTRY(release_entry)

SECTIONS {
 .text 0x08000100 : { *(.text*) *(.rodata*) }
 .data 0x20000000 : AT(0x08001000) { *(.data*) }
 .bss 0x20000004 (NOLOAD) : { *(.bss*) }
 /DISCARD/ : { *(.ARM.exidx*) *(.ARM.extab*) *(.comment*) }
}
