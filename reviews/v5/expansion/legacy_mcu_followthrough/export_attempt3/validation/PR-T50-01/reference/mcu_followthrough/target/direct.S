.syntax unified
.thumb
.fpu fpv4-sp-d16
.section .text.mf_direct_switch,"ax",%progbits
.global mf_direct_switch
.thumb_func
mf_direct_switch:
#if MF_FP
 vpush {s16-s31}
#endif
 push {r4-r11,lr}
 sub sp,sp,#4
#if MF_FP
 vmrs r2,fpscr
 str r2,[sp]
#endif
 str sp,[r0]
 mov sp,r1
#if MF_FP
 ldr r2,[sp]
 vmsr fpscr,r2
#endif
 add sp,sp,#4
 pop {r4-r11,lr}
#ifdef MF_BAD_REGISTER
 /* Deliberate first divergence: restored task loses r8, all else identical. */
 movs r2,#0
 mov r8,r2
#endif
#if MF_FP
 vpop {s16-s31}
#endif
 bx lr
.section .text.mf_direct_enter,"ax",%progbits
.global mf_direct_enter
.thumb_func
mf_direct_enter:
 mov r0,r4
 blx r5
 mov r0,r4
#ifdef MF_BAD_RETURN
 /* Deliberate return-policy defect, observed by the ordinary fault handler. */
 udf #19
#else
 bl mf_direct_return
#endif
 b mf_trap
.section .text.mf_direct_probe,"ax",%progbits
.global mf_direct_probe
.thumb_func
mf_direct_probe:
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
 ldr r1,[r0,#96]
 vmsr fpscr,r1
#endif
 ldmia r0,{r4-r11}
 bl mf_direct_yield
 ldr r0,[sp]
 add r1,r0,#100
 stmia r1!,{r4-r11}
#if MF_FP
 vstmia r1!,{s16-s31}
 vmrs r2,fpscr
 str r2,[r1]
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
