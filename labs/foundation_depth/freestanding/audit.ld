ENTRY(_start)
MEMORY { FLASH (rx) : ORIGIN = 0x08000000, LENGTH = 128K
         RAM (rwx) : ORIGIN = 0x20000000, LENGTH = 32K }
SECTIONS {
  .text : { *(.text*) *(.rodata*) } > FLASH
  .data : { *(.data*) } > RAM AT > FLASH
  .bss (NOLOAD) : { *(.bss*) *(COMMON) } > RAM
  /DISCARD/ : { *(.comment) *(.note*) *(.ARM.exidx*) *(.ARM.extab*) }
}
