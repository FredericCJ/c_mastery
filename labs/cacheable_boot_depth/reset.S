.syntax unified
.thumb
.section .vectors,"a",%progbits
.word _stack_top,cb_reset
.rept 164
.word cb_fault
.endr
.section .text,"ax",%progbits
.global cb_reset
.thumb_func
cb_reset:
 cpsid i
 ldr r0,=_vectors
 ldr r1,=0xE000ED08
 str r0,[r1]
 dsb sy
 isb sy
 ldr r0,=_data_load
 ldr r1,=_data_start
 ldr r2,=_data_end
1: cmp r1,r2
 bhs 2f
 ldr r3,[r0],#4
 str r3,[r1],#4
 b 1b
2: ldr r1,=_bss_start
 ldr r2,=_bss_end
 movs r3,#0
3: cmp r1,r2
 bhs 4f
 str r3,[r1],#4
 b 3b
4: bl cb_main
.thumb_func
cb_fault:
 b cb_fault
.section .note.GNU-stack,"",%progbits
