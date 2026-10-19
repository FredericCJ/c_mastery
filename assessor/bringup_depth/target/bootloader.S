.syntax unified
.thumb
.section .bootstrap,"ax",%progbits
.global bu_bootloader
.word _stack_top,bu_bootloader
.thumb_func
bu_bootloader:
 cpsid i
 /* Deliberately pending source and dirty SysTick, then bounded normalization. */
 ldr r0,=0xe000ed04
 ldr r1,=0x14000000
 str r1,[r0]
 ldr r1,=0x0a000000
 str r1,[r0]
 ldr r0,=0xe000e010
 movs r1,#0
 str r1,[r0]
 ldr r0,=0xe000e180
 mvns r1,r1
 .rept 3
 str r1,[r0],#4
 .endr
 ldr r0,=0xe000e280
 .rept 3
 str r1,[r0],#4
 .endr
 ldr r0,=0xe000ed08
 ldr r1,=bu_vectors_target
 str r1,[r0]
 dsb sy
 isb sy
 ldr r0,[r1]
 ldr r2,[r1,#4]
 mov sp,r0
 bx r2
.section .note.GNU-stack,"",%progbits
