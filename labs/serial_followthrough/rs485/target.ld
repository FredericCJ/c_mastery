ENTRY(xf_reset)
MEMORY { FLASH (rx) : ORIGIN = 0x00000000,LENGTH = 256K
 RAM (rwx) : ORIGIN = 0x20000000,LENGTH = 64K }
__stack_top = ORIGIN(RAM)+LENGTH(RAM);
SECTIONS {
 .vectors : ALIGN(1024) { KEEP(*(.vectors)) } >FLASH
 .text : { *(.text*) *(.rodata*) . = ALIGN(4); } >FLASH
 .data : ALIGN(4) { __data_start = .; *(.data*) . = ALIGN(4); __data_end = .; } >RAM AT>FLASH
 __data_load = LOADADDR(.data);
 .bss (NOLOAD) : ALIGN(8) { __bss_start = .; *(.bss*) *(COMMON) . = ALIGN(4); __bss_end = .; } >RAM
 .r5_dma (DEFINED(R5_DMA_BASE)?R5_DMA_BASE:ALIGN(__bss_end,4)) (NOLOAD) : { __r5_start = .; KEEP(*(.r5_dma)) __r5_end = .; }
 ASSERT(__r5_start>=0x20000000 && __r5_end<=0x20030000,"R5_DMA_REACHABILITY")
 ASSERT(__r5_end+8192<__stack_top,"handler stack reserve")
 ASSERT(SIZEOF(.vectors)==1024,"complete bounded vector table")
 /DISCARD/ : { *(.ARM.exidx*) *(.ARM.extab*) *(.comment) }
}
