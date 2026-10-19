ENTRY(Reset_Handler)
MEMORY { FLASH(rx) : ORIGIN = 0x00000000, LENGTH = 64
RAM(rwx) : ORIGIN = 0x20000000, LENGTH = 1024 }
SECTIONS {
.vectors : { KEEP(*(.vectors)) } > FLASH
.text : { *(.text*) *(.rodata*) } > FLASH
.ARM.exidx : { *(.ARM.exidx*) } > FLASH
.data : ALIGN(8) { _data_start = .; *(.data*) _data_end = .; } > RAM AT > FLASH
_data_load = LOADADDR(.data);
.bss (NOLOAD) : ALIGN(8) { _bss_start = .; *(.bss*) *(COMMON) _bss_end = .; } > RAM
.retained (NOLOAD) : ALIGN(8) { _retained_start = .; *(.retained*) _fixed_end = .; } > RAM
.heap (NOLOAD) : { . += 160; _heap_end = .; } > RAM
.guard (NOLOAD) : { . += 32; _guard_end = .; } > RAM
_stack_top = ORIGIN(RAM) + LENGTH(RAM); _stack_low = _stack_top - 256;
ASSERT(_fixed_end <= ORIGIN(RAM) + LENGTH(RAM), "fixed RAM overflow")
ASSERT(_guard_end <= _stack_low, "heap guard stack reservation overlap")
ASSERT(LOADADDR(.data) + SIZEOF(.data) <= ORIGIN(FLASH)+LENGTH(FLASH), "Flash load overflow")
/DISCARD/ : { *(.comment*) *(.note*) }
}
