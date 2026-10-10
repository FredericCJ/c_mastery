.syntax unified
.thumb
.section .vectors,"a",%progbits
.balign 512
.global cmx_vectors
cmx_vectors:
.word __stack_top
.word cmx_reset
.word cmx_fault
.word cmx_fault
.word cmx_fault
.word cmx_fault
.word cmx_fault
.word 0,0,0,0
.word SVC_Handler
.word cmx_fault
.word 0
.word PendSV_Handler
.word SysTick_Handler
.rept 125
.word cmx_fault
.endr
.word HSEM_Handler
.word HSEM_Handler
.rept 33
.word cmx_fault
.endr
.section .text.cmx_reset,"ax",%progbits
.global cmx_reset
.type cmx_reset,%function
.thumb_func
cmx_reset:
 cpsid i
 /* H745 CM4 private data/stack uses D2 SRAM1: enable its clock before C. */
#ifdef CMX_H745
 ldr r0,=0x580244dc
 ldr r1,[r0]
 orr r1,r1,#0xe0000000
 str r1,[r0]
 dsb
#endif
 ldr r0,=__data_load
 ldr r1,=__data_start
 ldr r2,=__data_end
1: cmp r1,r2
 bcs 2f
 ldr r3,[r0],#4
 str r3,[r1],#4
 b 1b
2: ldr r1,=__bss_start
 ldr r2,=__bss_end
 movs r3,#0
3: cmp r1,r2
 bcs 4f
 str r3,[r1],#4
 b 3b
4: ldr r0,=0xe000ed08
 ldr r1,=cmx_vectors
 str r1,[r0]
 dsb
 isb
 bl cmx_workload_main
 b cmx_fault
.size cmx_reset,.-cmx_reset
.section .note.GNU-stack,"",%progbits
