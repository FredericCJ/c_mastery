.syntax unified
.thumb
.fpu fpv4-sp-d16
.section .text.PendSV_Handler,"ax",%progbits
.global PendSV_Handler
.thumb_func
PendSV_Handler:
 cpsid i
 mrs r0,psp
 tst lr,#16
 it eq
 vstmdbeq r0!,{s16-s31}
 stmdb r0!,{r4-r11,lr}
 bl mf_choose
 ldmia r0!,{r4-r11,lr}
#ifdef MF_BAD_REGISTER
 movs r2,#0
 mov r8,r2
#endif
 tst lr,#16
 it eq
 vldmiaeq r0!,{s16-s31}
#ifdef MF_BAD_HIGH
 vmov.f32 s31,#1.0
#endif
#ifdef MF_BAD_EXC_RETURN
 /* Reserved Handler+PSP combination: explicit INVPC diagnostic, rather than
  * accidentally resuming an unrelated still-valid MSP frame. */
 ldr lr,=0xfffffff5
#endif
 msr psp,r0
 isb
 cpsie i
 bx lr
.section .text.SVC_Handler,"ax",%progbits
.global SVC_Handler
.thumb_func
SVC_Handler:
 cpsid i
 movs r0,#0
 bl mf_choose
 ldmia r0!,{r4-r11,lr}
 msr psp,r0
 movs r0,#2
 msr control,r0
 isb
 cpsie i
 bx lr
.section .text.SysTick_Handler,"ax",%progbits
.global SysTick_Handler
.thumb_func
SysTick_Handler:
 mov r0,lr
 mrs r1,psp
 mrs r2,msp
 b mf_tick
.section .text.NMI_Handler,"ax",%progbits
.global NMI_Handler
.thumb_func
NMI_Handler:
 mov r0,lr
 mrs r1,msp
 b mf_nmi
.section .text.UsageFault_Handler,"ax",%progbits
.global UsageFault_Handler,MemManage_Handler
.thumb_func
UsageFault_Handler:
.thumb_func
MemManage_Handler:
 tst lr,#4
 ite eq
 mrseq r0,msp
 mrsne r0,psp
 mov r1,lr
 b mf_local_fault
.section .text.mf_async_probe,"ax",%progbits
.global mf_async_probe
.thumb_func
mf_async_probe:
#if MF_FP
 vpush {s16-s31}
#endif
 push {r4-r11,lr}
 sub sp,sp,#12
 str r0,[sp]
#if MF_FP
 vmrs r1,fpscr
 str r1,[sp,#4]
 add r1,r0,#32
 vldmia r1!,{s16-s31}
 /* Distinct low register content is saved by the hardware frame. */
 add r1,r0,#32
 vldmia r1!,{s0-s15}
 ldr r1,[r0,#96]
 vmsr fpscr,r1
#endif
 ldmia r0,{r4-r11}
 ldr r0,=mf_ticks
 ldr r1,[r0]
 adds r1,r1,#4
1: ldr r2,[r0]
 cmp r2,r1
 blo 1b
 ldr r0,[sp]
 add r1,r0,#100
 stmia r1!,{r4-r11}
#if MF_FP
 vstmia r1!,{s16-s31}
 vmrs r2,fpscr
 str r2,[r1]
 /* Check every hardware-saved low FP word against the immutable pattern. */
 sub sp,sp,#64
 vstmia sp,{s0-s15}
 mov r1,sp
 add r2,r0,#32
 movs r3,#16
2: ldr r0,[r1],#4
 ldr r12,[r2],#4
 cmp r0,r12
 bne mf_trap
 subs r3,r3,#1
 bne 2b
 add sp,sp,#64
 ldr r0,[sp]
 ldr r1,=0x3f800000
 vmov s0,r1
 ldr r1,=0x33800000
 vmov s1,r1
 vadd.f32 s2,s0,s1
 vmov r1,s2
 str r1,[r0,#200]
 ldr r1,[sp,#4]
 vmsr fpscr,r1
#endif
 add sp,sp,#12
 pop {r4-r11,lr}
#if MF_FP
 vpop {s16-s31}
#endif
 bx lr
.section .note.GNU-stack,"",%progbits
