ENTRY(_start)
SECTIONS { . = 0x400000; .entry : { *(.text.entry) } . = 0x50000000; .far : { *(.text.far) } }
