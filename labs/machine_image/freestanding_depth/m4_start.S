.syntax unified
.thumb
.section .vectors,"a",%progbits
.word __stack_top
.word fr_reset
.rept 14
.word fr_fault
.endr
.text
.global fr_reset
.type fr_reset,%function
.thumb_func
fr_reset:
 ldr r0,=__data_load
 ldr r1,=__data_start
 ldr r2,=__data_end
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
4: bl target_main
 b fr_fault
.size fr_reset,.-fr_reset
.section .note.GNU-stack,"",%progbits
