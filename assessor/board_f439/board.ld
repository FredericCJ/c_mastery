/* NUCLEO-F439ZI board harness image (author physical-observation firmware).
 *
 * Every loadable byte lies in the first 64 KiB of main flash, so a program
 * operation erases only the sectors that tools/f439_board.py reports. RAM
 * stops below 0x2001F000; the 4 KiB above it holds the retained boot record,
 * which startup never copies or zeroes and which lies outside openocd's
 * 32 KiB flash-programming work area at 0x20000000.
 */
ENTRY(bf_reset)

MEMORY
{
    FLASH (rx)  : ORIGIN = 0x08000000, LENGTH = 64K
    RAM (rwx)   : ORIGIN = 0x20000000, LENGTH = 124K
    NOINIT (rw) : ORIGIN = 0x2001F000, LENGTH = 4K
}

_estack = ORIGIN(RAM) + LENGTH(RAM);

SECTIONS
{
    .vectors :
    {
        KEEP(*(.vectors))
    } > FLASH

    .text :
    {
        *(.text*)
        *(.rodata*)
        . = ALIGN(4);
    } > FLASH

    .data :
    {
        . = ALIGN(4);
        _sdata = .;
        *(.data*)
        . = ALIGN(4);
        _edata = .;
    } > RAM AT > FLASH
    _sidata = LOADADDR(.data);

    .bss (NOLOAD) :
    {
        . = ALIGN(4);
        _sbss = .;
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        _ebss = .;
    } > RAM

    .noinit (NOLOAD) :
    {
        . = ALIGN(4);
        KEEP(*(.noinit*))
        . = ALIGN(4);
    } > NOINIT

    /DISCARD/ :
    {
        *(.ARM.exidx*)
        *(.ARM.extab*)
    }

    ASSERT(ADDR(.vectors) == 0x08000000, "vector table must start flash")
    ASSERT(SIZEOF(.vectors) == 428, "vector table must hold 107 entries")
    ASSERT(_estack % 8 == 0, "AAPCS stack alignment")
    ASSERT(_ebss + 4096 <= _estack, "keep at least 4 KiB of stack")
}
