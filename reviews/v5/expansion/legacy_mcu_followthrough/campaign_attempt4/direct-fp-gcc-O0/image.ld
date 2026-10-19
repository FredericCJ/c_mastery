ENTRY(mf_reset)
MEMORY { FLASH(rx) : ORIGIN = 0,LENGTH = 1024K
 RAM(rwx) : ORIGIN = 0x20000000,LENGTH = 64K
 RAM2(rwx) : ORIGIN = 0x20020000,LENGTH = 64K }
__stack_top = ORIGIN(RAM)+LENGTH(RAM);
SECTIONS {
 .text : { KEEP(*(.vectors)) *(.text*) *(.rodata*) .  =  ALIGN(4); } >FLASH
 .data : { .  =  ALIGN(4); __data_start = .; *(.data) *(.data.*) .  =  ALIGN(4); __data_end = .; } >RAM AT>FLASH
 __data_load = LOADADDR(.data);
 .data2 : { .  =  ALIGN(4); __data2_start = .; *(.data2) .  =  ALIGN(4); __data2_end = .; } >RAM2 AT>FLASH
 __data2_load = LOADADDR(.data2);
 .bss (NOLOAD) : { .  =  ALIGN(8); __bss_start = .; *(.bss*) *(COMMON) .  =  ALIGN(8); __bss_end = .; } >RAM
 .retained (NOLOAD) : { __retained_start = .; KEEP(*(.retained)) __retained_end = .; } >RAM
 /DISCARD/ : { *(.ARM.exidx*) *(.ARM.extab*) *(.comment*) }
 ASSERT(__retained_end+8192<__stack_top,"handler stack budget exceeded")
}
