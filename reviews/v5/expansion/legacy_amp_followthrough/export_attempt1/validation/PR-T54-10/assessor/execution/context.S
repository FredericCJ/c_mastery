.syntax unified
.thumb
.fpu fpv4-sp-d16
.section .text.PendSV_Handler,"ax",%progbits
.global PendSV_Handler
.type PendSV_Handler,%function
.thumb_func
PendSV_Handler:
 cpsid i
 mrs r0,psp
 isb
 tst lr,#0x10
 it eq
 vstmdbeq r0!,{s16-s31}
 stmdb r0!,{r4-r11,lr}
 bl cmx_port_choose
 ldmia r0!,{r4-r11,lr}
 tst lr,#0x10
 it eq
 vldmiaeq r0!,{s16-s31}
 msr psp,r0
 isb
 cpsie i
 bx lr
.size PendSV_Handler,.-PendSV_Handler
.section .text.SVC_Handler,"ax",%progbits
.global SVC_Handler
.type SVC_Handler,%function
.thumb_func
SVC_Handler:
 cpsid i
 movs r0,#0
 bl cmx_port_choose
 ldmia r0!,{r4-r11,lr}
 msr psp,r0
 movs r0,#2
 msr control,r0
 isb
 cpsie i
 bx lr
.size SVC_Handler,.-SVC_Handler
.global cmx_fp_set,cmx_fp_get
.type cmx_fp_set,%function
.thumb_func
cmx_fp_set:
 vmov s16,r0
 bx lr
.type cmx_fp_get,%function
.thumb_func
cmx_fp_get:
 vmov r0,s16
 bx lr
.section .note.GNU-stack,"",%progbits
