ENTRY(release_entry)
PHDRS { text PT_LOAD FLAGS(5); data PT_LOAD FLAGS(6); }
SECTIONS {
 .text 0x08000100 : { *(.text*) *(.rodata*) } :text
 .data 0x20000000 : AT(0x08001000) { *(.data*) } :data
 .bss 0x20000004 (NOLOAD) : { *(.bss*) } :data
 /DISCARD/ : { *(.ARM.exidx*) *(.ARM.extab*) *(.comment*) }
}
