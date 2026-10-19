.syntax unified
.thumb
.section .vectors,"a",%progbits
.balign 512
.global g4_vectors
g4_vectors:
.word __stack_top,g4_reset,g4_fault,g4_fault,g4_fault,g4_fault,g4_fault
.word 0,0,0,0,g4_fault,g4_fault,0,g4_fault,g4_fault
/* No peripheral interrupts are enabled by this selected polling experiment. */
.section .text.g4_reset,"ax",%progbits
.global g4_reset
.type g4_reset,%function
.thumb_func
g4_reset:
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
4: ldr r0,=0xe000ed08
 ldr r1,=g4_vectors
 str r1,[r0]
 dsb
 isb
 bl g4_target_main
 b g4_fault
.size g4_reset,.-g4_reset
.section .text.g4_fault,"ax",%progbits
.global g4_fault
.type g4_fault,%function
.thumb_func
g4_fault:
 b g4_fault
.size g4_fault,.-g4_fault
.section .note.GNU-stack,"",%progbits
