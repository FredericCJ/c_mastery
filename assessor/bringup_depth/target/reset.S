.syntax unified
.thumb
.section .vectors,"a",%progbits
.balign 512
.global bu_vectors_target
bu_vectors_target:
.word _stack_top,bu_reset,bu_fault,bu_fault,bu_fault,bu_fault,bu_fault
.word 0,0,0,0,bu_svc,bu_fault,0,bu_fault,bu_tick
.rept 91
.word bu_fault
.endr
.section .text.reset,"ax",%progbits
.global bu_reset
.thumb_func
bu_reset:
 cpsid i
 ldr r0,=_stack_top
 mov sp,r0
 movs r0,#0
 msr control,r0
 msr basepri,r0
 msr faultmask,r0
 isb
 ldr r0,=bu_stage
 movs r1,#1
 str r1,[r0]
#if BU_F439
 ldr r0,=0x40023874
 ldr r1,[r0]
 ldr r0,=bu_reset_flags
 str r1,[r0]
#endif
 ldr r0,=0xe000ed08
 ldr r1,=bu_vectors_target
 str r1,[r0]
 dsb sy
 isb sy
#if BU_FP
 ldr r0,=0xe000ed88
 ldr r1,[r0]
 orr r1,r1,#0x00f00000
 str r1,[r0]
 dsb sy
 isb sy
#endif
 /* Seed otherwise unspecified RAM for discriminating zero-fill diagnosis. */
 ldr r4,=_zero_begin
 ldr r5,=_zero_end
1: cmp r4,r5
 bhs 4f
 ldm r4!,{r0,r1}
 ldr r2,=0xa5a5a5a5
2: cmp r0,r1
 bhs 1b
 str r2,[r0],#4
 b 2b
4: ldr r4,=_copy_begin
 ldr r5,=_copy_end
5: cmp r4,r5
 bhs 8f
 ldm r4!,{r0,r1,r2}
#if BU_BAD_LMA
 adds r0,#4
#endif
6: cmp r1,r2
 bhs 5b
 ldr r3,[r0],#4
 str r3,[r1],#4
 b 6b
8:
#if !BU_NO_ZERO
 ldr r4,=_zero_begin
 ldr r5,=_zero_end
9: cmp r4,r5
 bhs 12f
 ldm r4!,{r0,r1}
 movs r2,#0
10: cmp r0,r1
 bhs 9b
 str r2,[r0],#4
 b 10b
#endif
12: ldr r0,=bu_stage
 movs r1,#2
 str r1,[r0]
 ldr r0,=_stack_low
 mov r1,sp
 ldr r2,=0xa5a5a5a5
13: cmp r0,r1
 bhs 14f
 str r2,[r0],#4
 b 13b
14: bl bu_main
 /* A returning application has an explicit owner: diagnostic then stop. */
 ldr r0,=bu_stage
 movs r1,#5
 str r1,[r0]
 b bu_returned
.global bu_fault
.thumb_func
bu_fault:
 b bu_fault_report
.section .note.GNU-stack,"",%progbits
