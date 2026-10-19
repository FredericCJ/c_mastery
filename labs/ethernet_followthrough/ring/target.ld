ENTRY(xf_reset)
MEMORY { FLASH (rx) : ORIGIN = 0x00000000,LENGTH = 256K
 RAM (rwx) : ORIGIN = 0x20000000,LENGTH = 64K }
__stack_top = ORIGIN(RAM)+LENGTH(RAM);
__eth_base = DEFINED(ETH_BASE) ? ETH_BASE : 0x20001000;
SECTIONS {
 .vectors : ALIGN(1024) { KEEP(*(.vectors)) } >FLASH
 .text : { *(.text*) *(.rodata*) . = ALIGN(4); } >FLASH
 .data : ALIGN(4) { __data_start = .; *(.data*) . = ALIGN(4); __data_end = .; } >RAM AT>FLASH
 __data_load = LOADADDR(.data);
 .eth __eth_base (NOLOAD) : ALIGN(4) { KEEP(*(.eth)) } >RAM
 .bss (NOLOAD) : ALIGN(8) { __bss_start = .; *(.bss*) *(COMMON) . = ALIGN(4); __bss_end = .; } >RAM
 ASSERT(ADDR(.eth)>=0x20000000 && ADDR(.eth)+SIZEOF(.eth)<=0x20030000,"EN_DMA_REACHABILITY")
 ASSERT(SIZEOF(.eth)==12384 && (ADDR(.eth)&3)==0,"EN_NORMAL_LAYOUT")
 ASSERT(__bss_end+16384<__stack_top,"EN verification stack reserve")
 ASSERT(SIZEOF(.vectors)==1024,"complete bounded vector table")
 /DISCARD/ : { *(.ARM.exidx*) *(.ARM.extab*) *(.comment) }
}
