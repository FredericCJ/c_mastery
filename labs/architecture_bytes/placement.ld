MEMORY { CCM (rw) : ORIGIN = 0x10000000, LENGTH = 64K
 SRAM (rw) : ORIGIN = 0x20000000, LENGTH = 112K }
SECTIONS { .ccm (NOLOAD) : ALIGN(8) { *(.ccm) } > CCM
 .sram (NOLOAD) : ALIGN(8) { *(.sram) } > SRAM
 /DISCARD/ : { *(.comment) *(.ARM.attributes) } }
