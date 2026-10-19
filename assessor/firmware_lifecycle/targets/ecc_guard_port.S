.syntax unified
.thumb
.text
.global eg_raw,eg_load_insn,eg_abort,eg_busfault
.type eg_raw,%function
.thumb_func
eg_raw:
eg_load_insn:
 ldr r2,[r0]
 str r2,[r1]
 movs r0,#1
 bx lr
eg_abort:
 movs r0,#0
 bx lr
.type eg_busfault,%function
.thumb_func
eg_busfault:
 tst lr,#4
 ite eq
 mrseq r0,msp
 mrsne r0,psp
 mov r1,lr
 push {r4,lr}
 bl eg_handle
 cmp r0,#1
 bne 1f
 pop {r4,lr}
 bx lr
1: bl eg_unhandled
 b .
.section .note.GNU-stack,"",%progbits
