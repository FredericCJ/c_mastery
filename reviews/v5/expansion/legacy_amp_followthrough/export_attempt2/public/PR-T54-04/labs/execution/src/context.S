.syntax unified
.thumb
.section .text,"ax",%progbits
.global PendSV_Handler,SVC_Handler,cmx_fp_set,cmx_fp_get
.thumb_func
PendSV_Handler:
 b cmx_fault
.thumb_func
SVC_Handler:
 b cmx_fault
.thumb_func
cmx_fp_set:
 bx lr
.thumb_func
cmx_fp_get:
 movs r0,#0
 bx lr
.section .note.GNU-stack,"",%progbits
