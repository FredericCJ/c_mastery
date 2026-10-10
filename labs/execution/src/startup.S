.syntax unified
.thumb
.section .vectors,"a",%progbits
.balign 512
.global cmx_vectors
cmx_vectors:
.word __stack_top
.word cmx_reset
.rept 9
.word cmx_fault
.endr
.word SVC_Handler
.word cmx_fault,0,PendSV_Handler,SysTick_Handler
.rept 160
.word cmx_fault
.endr
.section .text,"ax",%progbits
.global cmx_reset
.thumb_func
cmx_reset:
 /* TODO: own data/bss, VTOR, target clocks, then enter original C. */
 b cmx_fault
.section .note.GNU-stack,"",%progbits
