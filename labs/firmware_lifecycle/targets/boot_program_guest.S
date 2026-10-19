.syntax unified
.thumb
.section .vectors,"a",%progbits
.word 0x20008000,fp_reset,fp_fail,fp_fail,fp_memfault
.rept 11
.word fp_fail
.endr
.section .text,"ax",%progbits
.global fp_reset,fp_raw,fp_store,fp_abort
.thumb_func
fp_reset:
 bl fp_main
 b fp_fail
.thumb_func
fp_raw:
fp_store:
 str r1,[r0]
 movs r0,#1
 bx lr
fp_abort:
 movs r0,#0
 bx lr
.thumb_func
fp_memfault:
 mov r0,sp
 mov r1,lr
 push {r4,lr}
 bl fp_handler
 pop {r4,lr}
 bx lr
.thumb_func
fp_fail:
 b fp_fail
.section .note.GNU-stack,"",%progbits
