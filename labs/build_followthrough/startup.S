.syntax unified
.cpu cortex-m4
.thumb
.section .vectors,"a",%progbits
.word 0x20020000
.word Reset_Handler
.section .text.Reset_Handler,"ax",%progbits
.global Reset_Handler
.type Reset_Handler,%function
.thumb_func
Reset_Handler:
 cpsid i
 bl reset
1: b 1b
