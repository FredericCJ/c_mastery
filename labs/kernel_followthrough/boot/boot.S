.section .multiboot,"a"
.align 8
header:
.long 0xe85250d6,0,24,(-(0xe85250d6+24)) & 0xffffffff
.short 0,0
.long 8
.section .text.boot,"ax"
.code32
.global _start
_start:
 cli
 /* PR-T48-01: preserve boot arguments, create page tables/GDT, enter long mode. */
1: hlt
 jmp 1b
.section .note.GNU-stack,"",@progbits
