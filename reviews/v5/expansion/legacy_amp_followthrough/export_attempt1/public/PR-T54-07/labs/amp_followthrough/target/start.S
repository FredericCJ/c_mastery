.syntax unified
.thumb
.section .vectors,"a",%progbits
.global af_vectors
af_vectors:
.word __stack_top, af_reset
.rept 9
.word af_fault
.endr
#ifdef AF_PIPELINE
.word SVC_Handler, af_fault, af_fault, PendSV_Handler, SysTick_Handler
#else
.word af_fault, af_fault, af_fault, af_fault
.word af_systick
#endif
.rept 125
.word af_fault
.endr
#if AF_M7
.word af_hsem, af_fault
#else
.word af_fault, af_hsem
#endif
.text
.global af_reset
.type af_reset,%function
.thumb_func
af_reset:
 cpsid i
 ldr r0, =__data_load
 ldr r1, =__data_start
 ldr r2, =__data_end
1: cmp r1,r2
 bhs 2f
 ldr r3,[r0],#4
 str r3,[r1],#4
 b 1b
2: ldr r1,=__bss_start
 ldr r2,=__bss_end
 movs r3,#0
3: cmp r1,r2
 bhs 4f
 str r3,[r1],#4
 b 3b
4: bl af_entry
 b .
.size af_reset,.-af_reset
