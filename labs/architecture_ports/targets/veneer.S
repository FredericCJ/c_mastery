.section .text.entry,"ax"
.global _start
_start:
 bl far_function
 ret
.section .text.far,"ax"
.global far_function
far_function:
 ret
