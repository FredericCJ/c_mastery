.syntax unified
.thumb
.section .vectors,"a",%progbits
.balign 512
.global cp_vectors
cp_vectors:
.word __stack_top,cp_reset,cp_fault,cp_fault,cp_fault,cp_fault,cp_fault
.word 0,0,0,0,cp_fault,cp_fault,0,cp_pendsv,cp_fault
.rept 6
.word cp_fault
.endr
.word p4_exti0
.rept 89
.word cp_fault
.endr
.section .text.cp_reset,"ax",%progbits
.global cp_reset
.type cp_reset,%function
.thumb_func
cp_reset:
 cpsid i
#if CP_MUTANT != 41
 ldr r0,=__data_load
 ldr r1,=__data_start
 ldr r2,=__data_end
1: cmp r1,r2
 bcs 2f
 ldr r3,[r0],#4
 str r3,[r1],#4
 b 1b
2:
#endif
#if CP_MUTANT != 42
 ldr r1,=__bss_start
 ldr r2,=__bss_end
 movs r3,#0
3: cmp r1,r2
 bcs 4f
 str r3,[r1],#4
 b 3b
4:
#endif
 ldr r0,=0xe000ed08
 ldr r1,=cp_vectors
 str r1,[r0]
 dsb
 isb
 bl cp_target_main
 b cp_fault
.size cp_reset,.-cp_reset
.section .note.GNU-stack,"",%progbits
