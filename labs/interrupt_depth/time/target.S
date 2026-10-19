.syntax unified
.thumb
.section .vectors,"a",%progbits
.balign 1024
.word __stack_top,xf_reset
.rept 22
.word xf_fault
.endr
.word xk_timer
.rept 231
.word xf_fault
.endr
.section .text.xf_reset,"ax",%progbits
.global xf_reset
.type xf_reset,%function
.thumb_func
xf_reset:
 cpsid i
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
4: bl xf_target_main
 b xf_fault
.size xf_reset,.-xf_reset
.section .note.GNU-stack,"",%progbits
