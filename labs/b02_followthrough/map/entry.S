.syntax unified
.cpu cortex-m4
.thumb
.section .vectors,"a"
.word 0x20040000
.word Reset_Handler
.rept 14
.word Fault_Handler
.endr
.section .text.Reset_Handler,"ax"
.global Reset_Handler
.thumb_func
Reset_Handler:
 ldr r0,=_sdata
 ldr r1,=_edata
 ldr r2,=_sidata
1: cmp r0,r1
 bhs 2f
 ldr r3,[r2],#4
 str r3,[r0],#4
 b 1b
2: ldr r0,=_sbss
 ldr r1,=_ebss
 movs r2,#0
3: cmp r0,r1
 bhs 4f
 str r2,[r0],#4
 b 3b
4: bl bf_target
 b .
.thumb_func
Fault_Handler:
 b .
