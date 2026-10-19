ENTRY(th_primary)
SECTIONS { . = 0x08000000; .vectors : { KEEP(*(.vectors)) } .text : { *(.text*) *(.rodata*) } ASSERT(. < 0x08020000,"primary crosses bootstrap sector") /DISCARD/ : { *(.comment) *(.ARM.exidx*) *(.ARM.extab*) } }
