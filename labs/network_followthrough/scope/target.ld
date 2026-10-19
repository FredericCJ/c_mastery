ENTRY(xf_reset)
/* Executable MPS2 memory map. F439 placement audit separately relocates FLASH;
 * this image does not configure a physical PHY, DMA or board clock. */
MEMORY { FLASH (rx) : ORIGIN = 0,LENGTH = 256K
 DMA (rwx) : ORIGIN = 0x20000000,LENGTH = 32K
 RAM (rwx) : ORIGIN = 0x20008000,LENGTH = 96K }
__stack_top = ORIGIN(RAM)+LENGTH(RAM);
SECTIONS {
 .vectors : ALIGN(1024) { KEEP(*(.vectors)) } >FLASH
 .text : { *(.text*) *(.rodata*) . = ALIGN(4); } >FLASH
 .data : ALIGN(4) { __data_start = .; *(.data*) . = ALIGN(4); __data_end = .; } >RAM AT>FLASH
 __data_load = LOADADDR(.data);
 .dma (NOLOAD) : ALIGN(32) { __dma_start = .; KEEP(*(.dma)) . = ALIGN(32); __dma_end = .; } >DMA
 .bss (NOLOAD) : ALIGN(8) { __bss_start = .; *(.bss*) *(COMMON) . = ALIGN(4); __bss_end = .; } >RAM
 ASSERT(__dma_end-__dma_start==12384,"six buffers and six descriptors")
 ASSERT(__bss_end+32768<__stack_top,"explicit parser/call stack reserve")
 ASSERT(SIZEOF(.vectors)==1024,"complete vector table")
 /DISCARD/ : { *(.ARM.exidx*) *(.ARM.extab*) *(.comment) }
}
