.syntax unified
.thumb
.section .vectors,"a",%progbits
.balign 1024
.global st_vectors
st_vectors:
.word _stack_top,st_reset,st_fault,st_fault,st_fault,eg_busfault
.rept 160
.word st_fault
.endr
.section .text.start,"ax",%progbits
.global st_reset
.thumb_func
st_reset:
 ldr r0,=0xE000ED08
 ldr r1,=st_vectors
 str r1,[r0]
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
4: bl st_main
 b st_fault
.thumb_func
st_fault:
 wfi
 b st_fault
.section .note.GNU-stack,"",%progbits
