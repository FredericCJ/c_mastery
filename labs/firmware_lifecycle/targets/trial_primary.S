.syntax unified
.thumb
.section .vectors,"a",%progbits
.global th_primary_vectors
th_primary_vectors:
.word 0x20020000,th_primary
.rept 164
.word th_stop
.endr
.section .text,"ax",%progbits
.global th_primary
.thumb_func
th_primary:
 ldr r2,=0x08120000
 ldr r0,[r2]
 ldr r1,[r2,#4]
 ldr r3,=0xE000ED08
 str r2,[r3]
 dsb sy
 isb sy
 msr msp,r0
 bx r1
.thumb_func
th_stop: wfi
 b th_stop
.section .note.GNU-stack,"",%progbits
