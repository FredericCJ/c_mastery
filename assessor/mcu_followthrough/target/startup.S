.syntax unified
.thumb
.section .vectors,"a",%progbits
.balign 512
.global mf_vectors
mf_vectors:
.word __stack_top,mf_reset,NMI_Handler,mf_trap
.word MemManage_Handler,mf_trap,UsageFault_Handler,0,0,0,0
.word SVC_Handler,mf_trap,0,PendSV_Handler,SysTick_Handler
.word UART0_Handler
.rept 7
.word mf_trap
.endr
.word Timer0_Handler,Timer1_Handler
.rept 54
.word mf_trap
.endr
.section .text.mf_reset,"ax",%progbits
.global mf_reset
.thumb_func
mf_reset:
 cpsid i
 ldr r0,=__data_load
#ifdef MF_BAD_LMA
 adds r0,r0,#4
#endif
 ldr r1,=__data_start
 ldr r2,=__data_end
 bl mf_copy_words
 ldr r0,=__data2_load
 ldr r1,=__data2_start
 ldr r2,=__data2_end
 bl mf_copy_words
#ifndef MF_BAD_BSS
 ldr r1,=__bss_start
 ldr r2,=__bss_end
 movs r3,#0
1: cmp r1,r2
 bcs 2f
 str r3,[r1],#4
 b 1b
2:
#endif
 ldr r0,=0xe000ed08
 ldr r1,=mf_vectors
 str r1,[r0]
 ldr r0,=0xe000ed88
 ldr r1,[r0]
 orr r1,r1,#0x00f00000
 str r1,[r0]
 dsb
 isb
 bl mf_main
 b mf_trap
.thumb_func
mf_copy_words:
 cmp r1,r2
 bcs 2f
1: ldr r3,[r0],#4
 str r3,[r1],#4
 cmp r1,r2
 bcc 1b
2: bx lr
.macro default_handler name
 .section .text.\name,"ax",%progbits
 .weak \name
 .thumb_func
\name:
 b mf_trap
.endm
default_handler NMI_Handler
default_handler MemManage_Handler
default_handler UsageFault_Handler
default_handler SVC_Handler
default_handler PendSV_Handler
default_handler SysTick_Handler
default_handler Timer0_Handler
default_handler Timer1_Handler
default_handler UART0_Handler
.section .note.GNU-stack,"",%progbits
