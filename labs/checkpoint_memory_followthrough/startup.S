.syntax unified
.thumb
.section .vectors,"a",%progbits
.balign 1024
.word __stack_top,p1_reset
.rept 22
.word p1_fault
.endr
.word p1_fault
.rept 231
.word p1_fault
.endr
.section .text.p1_reset,"ax",%progbits
.global p1_reset
.type p1_reset,%function
.thumb_func
p1_reset:
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
4: bl p1_target_main
 b p1_fault
.size p1_reset,.-p1_reset
.section .note.GNU-stack,"",%progbits
