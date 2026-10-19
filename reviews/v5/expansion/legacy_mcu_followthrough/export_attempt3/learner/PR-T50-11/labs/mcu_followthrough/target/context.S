.syntax unified
.thumb
.section .text.PendSV_Handler,"ax",%progbits
.global PendSV_Handler,SVC_Handler,mf_async_probe
.thumb_func
PendSV_Handler:
 b mf_trap
.thumb_func
SVC_Handler:
 b mf_trap
.thumb_func
mf_async_probe:
 b mf_trap
.section .note.GNU-stack,"",%progbits
