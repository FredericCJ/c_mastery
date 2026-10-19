.syntax unified
.thumb
.section .vectors,"a",%progbits
.balign 512
.word __stack_top,mf_reset
.rept 78
.word mf_trap
.endr
.section .text.mf_reset,"ax",%progbits
.global mf_reset
.thumb_func
mf_reset:
 /* TODO: vector routing, initialized regions, zero range and retained range. */
 b mf_trap
.section .note.GNU-stack,"",%progbits
