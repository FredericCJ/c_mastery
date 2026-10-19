/* Audit link only. Merge both metadata reservations into the B06 boot images. */
MEMORY {
 FLASH (rx) : ORIGIN = 0x08000000, LENGTH = 1M
 RAM (rwx) : ORIGIN = 0x20000000, LENGTH = 128K
 SHARED (rw) : ORIGIN = 0x30040000, LENGTH = 32K
}
SECTIONS {
 .text : { *(.text*) *(.rodata*) } > FLASH
 .ARM.exidx : { *(.ARM.exidx*) } > FLASH
 .data : { *(.data*) } > RAM AT> FLASH
 .bss (NOLOAD) : { *(.bss*) *(COMMON) } > RAM
 .pd_shared (NOLOAD) : { __pd_shared_start = .; . += 64; __pd_shared_end = .; } > SHARED
}
ASSERT(__pd_shared_start == 0x30040000,"Peer metadata must start at SRAM3 base")
ASSERT(__pd_shared_end == 0x30040040,"Two separate whole metadata lines reserved")
