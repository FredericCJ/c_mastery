SECTIONS { . = 0x10000; .text : { *(.text*) } .bss : { *(.bss*) *(COMMON) } ASSERT(SIZEOF(.bss) <= 64, "BSS budget exceeded") }
