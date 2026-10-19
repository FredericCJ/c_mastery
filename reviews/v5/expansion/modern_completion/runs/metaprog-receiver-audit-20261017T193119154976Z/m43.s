	.syntax	unified
	.eabi_attribute	67, "2.09"	@ Tag_conformance
	.cpu	cortex-m4
	.eabi_attribute	6, 13	@ Tag_CPU_arch
	.eabi_attribute	7, 77	@ Tag_CPU_arch_profile
	.eabi_attribute	8, 0	@ Tag_ARM_ISA_use
	.eabi_attribute	9, 2	@ Tag_THUMB_ISA_use
	.eabi_attribute	34, 1	@ Tag_CPU_unaligned_access
	.eabi_attribute	17, 1	@ Tag_ABI_PCS_GOT_use
	.eabi_attribute	20, 1	@ Tag_ABI_FP_denormal
	.eabi_attribute	21, 0	@ Tag_ABI_FP_exceptions
	.eabi_attribute	23, 3	@ Tag_ABI_FP_number_model
	.eabi_attribute	24, 1	@ Tag_ABI_align_needed
	.eabi_attribute	25, 1	@ Tag_ABI_align_preserved
	.eabi_attribute	38, 1	@ Tag_ABI_FP_16bit_format
	.eabi_attribute	18, 4	@ Tag_ABI_PCS_wchar_t
	.eabi_attribute	26, 2	@ Tag_ABI_enum_size
	.eabi_attribute	14, 0	@ Tag_ABI_PCS_R9_use
	.file	"receivers.c"
	.text
	.globl	mpv_read_m                      @ -- Begin function mpv_read_m
	.p2align	2
	.type	mpv_read_m,%function
	.code	16
	.thumb_func
mpv_read_m:                             @ @mpv_read_m
	.fnstart
@ %bb.0:
	mov	r2, r0
	cmp	r0, #0
	mov.w	r0, #1
	it	ne
	cmpne	r1, #0
	bne	.LBB0_2
@ %bb.1:
	bx	lr
	.p2align	2
.LBB0_2:
	ldr	r0, [r2]
	movs	r2, #1
	strd	r0, r2, [r1]
	movs	r0, #0
	bx	lr
.Lfunc_end0:
	.size	mpv_read_m, .Lfunc_end0-mpv_read_m
	.cantunwind
	.fnend
                                        @ -- End function
	.globl	mpv_read_c                      @ -- Begin function mpv_read_c
	.p2align	2
	.type	mpv_read_c,%function
	.code	16
	.thumb_func
mpv_read_c:                             @ @mpv_read_c
	.fnstart
@ %bb.0:
	mov	r2, r0
	cmp	r0, #0
	mov.w	r0, #1
	it	ne
	cmpne	r1, #0
	bne	.LBB1_2
@ %bb.1:
	bx	lr
	.p2align	2
.LBB1_2:
	ldr	r0, [r2]
	movs	r2, #2
	strd	r0, r2, [r1]
	movs	r0, #0
	bx	lr
.Lfunc_end1:
	.size	mpv_read_c, .Lfunc_end1-mpv_read_c
	.cantunwind
	.fnend
                                        @ -- End function
	.globl	mpv_read_v                      @ -- Begin function mpv_read_v
	.p2align	2
	.type	mpv_read_v,%function
	.code	16
	.thumb_func
mpv_read_v:                             @ @mpv_read_v
	.fnstart
@ %bb.0:
	mov	r2, r0
	cmp	r0, #0
	mov.w	r0, #1
	it	ne
	cmpne	r1, #0
	bne	.LBB2_2
@ %bb.1:
	bx	lr
	.p2align	2
.LBB2_2:
	ldr	r0, [r2]
	movs	r2, #3
	strd	r0, r2, [r1]
	movs	r0, #0
	bx	lr
.Lfunc_end2:
	.size	mpv_read_v, .Lfunc_end2-mpv_read_v
	.cantunwind
	.fnend
                                        @ -- End function
	.globl	mpv_read_cv                     @ -- Begin function mpv_read_cv
	.p2align	2
	.type	mpv_read_cv,%function
	.code	16
	.thumb_func
mpv_read_cv:                            @ @mpv_read_cv
	.fnstart
@ %bb.0:
	mov	r2, r0
	cmp	r0, #0
	mov.w	r0, #1
	it	ne
	cmpne	r1, #0
	bne	.LBB3_2
@ %bb.1:
	bx	lr
	.p2align	2
.LBB3_2:
	ldr	r0, [r2]
	movs	r2, #4
	strd	r0, r2, [r1]
	movs	r0, #0
	bx	lr
.Lfunc_end3:
	.size	mpv_read_cv, .Lfunc_end3-mpv_read_cv
	.cantunwind
	.fnend
                                        @ -- End function
	.globl	mpv_write_m                     @ -- Begin function mpv_write_m
	.p2align	2
	.type	mpv_write_m,%function
	.code	16
	.thumb_func
mpv_write_m:                            @ @mpv_write_m
	.fnstart
@ %bb.0:
	cmp	r0, #0
	itt	eq
	moveq	r0, #1
	bxeq	lr
.LBB4_1:
	mvns	r1, r1
	tst	r2, r1
	itee	ne
	movne	r0, #2
	streq	r2, [r0]
	moveq	r0, #0
	bx	lr
.Lfunc_end4:
	.size	mpv_write_m, .Lfunc_end4-mpv_write_m
	.cantunwind
	.fnend
                                        @ -- End function
	.globl	mpv_write_v                     @ -- Begin function mpv_write_v
	.p2align	2
	.type	mpv_write_v,%function
	.code	16
	.thumb_func
mpv_write_v:                            @ @mpv_write_v
	.fnstart
@ %bb.0:
	cmp	r0, #0
	itt	eq
	moveq	r0, #1
	bxeq	lr
.LBB5_1:
	mvns	r1, r1
	tst	r2, r1
	itee	ne
	movne	r0, #2
	streq	r2, [r0]
	moveq	r0, #0
	bx	lr
.Lfunc_end5:
	.size	mpv_write_v, .Lfunc_end5-mpv_write_v
	.cantunwind
	.fnend
                                        @ -- End function
	.globl	mpv_observe_v                   @ -- Begin function mpv_observe_v
	.p2align	2
	.type	mpv_observe_v,%function
	.code	16
	.thumb_func
mpv_observe_v:                          @ @mpv_observe_v
	.fnstart
@ %bb.0:
	.save	{r4, r5, r6, lr}
	push	{r4, r5, r6, lr}
	mov	r5, r0
	cmp	r0, #0
	mov.w	r0, #1
	it	ne
	cmpne	r1, #0
	bne	.LBB6_2
@ %bb.1:
	pop	{r4, r5, r6, pc}
	.p2align	2
.LBB6_2:
	mov	r4, r3
	cmp	r3, #0
	it	eq
	popeq	{r4, r5, r6, pc}
.LBB6_3:
	ldr	r6, [r5]
	mov	r0, r2
	blx	r1
	ldr	r0, [r5]
	strd	r6, r0, [r4]
	movs	r0, #0
	pop	{r4, r5, r6, pc}
.Lfunc_end6:
	.size	mpv_observe_v, .Lfunc_end6-mpv_observe_v
	.cantunwind
	.fnend
                                        @ -- End function
	.globl	mpv_observe_cv                  @ -- Begin function mpv_observe_cv
	.p2align	2
	.type	mpv_observe_cv,%function
	.code	16
	.thumb_func
mpv_observe_cv:                         @ @mpv_observe_cv
	.fnstart
@ %bb.0:
	.save	{r4, r5, r6, lr}
	push	{r4, r5, r6, lr}
	mov	r5, r0
	cmp	r0, #0
	mov.w	r0, #1
	it	ne
	cmpne	r1, #0
	bne	.LBB7_2
@ %bb.1:
	pop	{r4, r5, r6, pc}
	.p2align	2
.LBB7_2:
	mov	r4, r3
	cmp	r3, #0
	it	eq
	popeq	{r4, r5, r6, pc}
.LBB7_3:
	ldr	r6, [r5]
	mov	r0, r2
	blx	r1
	ldr	r0, [r5]
	strd	r6, r0, [r4]
	movs	r0, #0
	pop	{r4, r5, r6, pc}
.Lfunc_end7:
	.size	mpv_observe_cv, .Lfunc_end7-mpv_observe_cv
	.cantunwind
	.fnend
                                        @ -- End function
	.globl	mpv_update                      @ -- Begin function mpv_update
	.p2align	2
	.type	mpv_update,%function
	.code	16
	.thumb_func
mpv_update:                             @ @mpv_update
	.fnstart
@ %bb.0:
	cmp	r0, #0
	itt	eq
	moveq	r0, #1
	bxeq	lr
.LBB8_1:
	orr.w	r12, r3, r2
	mvns	r1, r1
	tst.w	r12, r1
	itt	ne
	movne	r0, #2
	bxne	lr
.LBB8_2:
	ldr	r1, [r0]
	bics	r1, r2
	orrs	r1, r3
	str	r1, [r0]
	movs	r0, #0
	bx	lr
.Lfunc_end8:
	.size	mpv_update, .Lfunc_end8-mpv_update
	.cantunwind
	.fnend
                                        @ -- End function
	.globl	mpv_ack                         @ -- Begin function mpv_ack
	.p2align	2
	.type	mpv_ack,%function
	.code	16
	.thumb_func
mpv_ack:                                @ @mpv_ack
	.fnstart
@ %bb.0:
	cmp	r0, #0
	itt	eq
	moveq	r0, #1
	bxeq	lr
.LBB9_1:
	mvns	r1, r1
	tst	r2, r1
	itee	ne
	movne	r0, #2
	streq	r2, [r0]
	moveq	r0, #0
	bx	lr
.Lfunc_end9:
	.size	mpv_ack, .Lfunc_end9-mpv_ack
	.cantunwind
	.fnend
                                        @ -- End function
	.ident	"Ubuntu clang version 21.1.8 (6ubuntu1)"
	.section	".note.GNU-stack","",%progbits
	.addrsig
	.eabi_attribute	30, 2	@ Tag_ABI_optimization_goals
