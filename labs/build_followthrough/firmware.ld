ENTRY(Reset_Handler)
MEMORY { FLASH (rx) : ORIGIN = 0x08000000, LENGTH = 1024K
         RAM (rwx) : ORIGIN = 0x20000000, LENGTH = 128K }
SECTIONS {
 .vectors : { KEEP(*(.vectors)) } > FLASH
 .text : { *(.text*) *(.rodata*) } > FLASH
 .ARM.exidx : { *(.ARM.exidx*) } > FLASH
 .bss (NOLOAD) : { __bss_start__ = .; *(.bss*) *(COMMON) __bss_end__ = .; } > RAM
 /DISCARD/ : { *(.comment) }
}
