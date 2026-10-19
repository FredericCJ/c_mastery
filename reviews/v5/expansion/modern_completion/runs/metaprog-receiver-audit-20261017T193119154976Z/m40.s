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
	.pad	#16
	sub	sp, #16
	str	r0, [sp, #8]
	str	r1, [sp, #4]
	ldr	r0, [sp, #8]
	cbz	r0, .LBB0_2
	b	.LBB0_1
.LBB0_1:
	ldr	r0, [sp, #4]
	cbnz	r0, .LBB0_3
	b	.LBB0_2
.LBB0_2:
	movs	r0, #1
	str	r0, [sp, #12]
	b	.LBB0_4
.LBB0_3:
	ldr	r0, [sp, #8]
	ldr	r0, [r0]
	str	r0, [sp]
	ldr	r0, [sp]
	ldr	r1, [sp, #4]
	str	r0, [r1]
	ldr	r1, [sp, #4]
	movs	r0, #1
	str	r0, [r1, #4]
	movs	r0, #0
	str	r0, [sp, #12]
	b	.LBB0_4
.LBB0_4:
	ldr	r0, [sp, #12]
	add	sp, #16
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
	.pad	#16
	sub	sp, #16
	str	r0, [sp, #8]
	str	r1, [sp, #4]
	ldr	r0, [sp, #8]
	cbz	r0, .LBB1_2
	b	.LBB1_1
.LBB1_1:
	ldr	r0, [sp, #4]
	cbnz	r0, .LBB1_3
	b	.LBB1_2
.LBB1_2:
	movs	r0, #1
	str	r0, [sp, #12]
	b	.LBB1_4
.LBB1_3:
	ldr	r0, [sp, #8]
	ldr	r0, [r0]
	str	r0, [sp]
	ldr	r0, [sp]
	ldr	r1, [sp, #4]
	str	r0, [r1]
	ldr	r1, [sp, #4]
	movs	r0, #2
	str	r0, [r1, #4]
	movs	r0, #0
	str	r0, [sp, #12]
	b	.LBB1_4
.LBB1_4:
	ldr	r0, [sp, #12]
	add	sp, #16
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
	.pad	#16
	sub	sp, #16
	str	r0, [sp, #8]
	str	r1, [sp, #4]
	ldr	r0, [sp, #8]
	cbz	r0, .LBB2_2
	b	.LBB2_1
.LBB2_1:
	ldr	r0, [sp, #4]
	cbnz	r0, .LBB2_3
	b	.LBB2_2
.LBB2_2:
	movs	r0, #1
	str	r0, [sp, #12]
	b	.LBB2_4
.LBB2_3:
	ldr	r0, [sp, #8]
	ldr	r0, [r0]
	str	r0, [sp]
	ldr	r0, [sp]
	ldr	r1, [sp, #4]
	str	r0, [r1]
	ldr	r1, [sp, #4]
	movs	r0, #3
	str	r0, [r1, #4]
	movs	r0, #0
	str	r0, [sp, #12]
	b	.LBB2_4
.LBB2_4:
	ldr	r0, [sp, #12]
	add	sp, #16
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
	.pad	#16
	sub	sp, #16
	str	r0, [sp, #8]
	str	r1, [sp, #4]
	ldr	r0, [sp, #8]
	cbz	r0, .LBB3_2
	b	.LBB3_1
.LBB3_1:
	ldr	r0, [sp, #4]
	cbnz	r0, .LBB3_3
	b	.LBB3_2
.LBB3_2:
	movs	r0, #1
	str	r0, [sp, #12]
	b	.LBB3_4
.LBB3_3:
	ldr	r0, [sp, #8]
	ldr	r0, [r0]
	str	r0, [sp]
	ldr	r0, [sp]
	ldr	r1, [sp, #4]
	str	r0, [r1]
	ldr	r1, [sp, #4]
	movs	r0, #4
	str	r0, [r1, #4]
	movs	r0, #0
	str	r0, [sp, #12]
	b	.LBB3_4
.LBB3_4:
	ldr	r0, [sp, #12]
	add	sp, #16
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
	.pad	#16
	sub	sp, #16
	str	r0, [sp, #8]
	str	r1, [sp, #4]
	str	r2, [sp]
	ldr	r0, [sp, #8]
	cbnz	r0, .LBB4_2
	b	.LBB4_1
.LBB4_1:
	movs	r0, #1
	str	r0, [sp, #12]
	b	.LBB4_5
.LBB4_2:
	ldr	r0, [sp]
	ldr	r1, [sp, #4]
	mvns	r1, r1
	tst	r0, r1
	beq	.LBB4_4
	b	.LBB4_3
.LBB4_3:
	movs	r0, #2
	str	r0, [sp, #12]
	b	.LBB4_5
.LBB4_4:
	ldr	r0, [sp]
	ldr	r1, [sp, #8]
	str	r0, [r1]
	movs	r0, #0
	str	r0, [sp, #12]
	b	.LBB4_5
.LBB4_5:
	ldr	r0, [sp, #12]
	add	sp, #16
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
	.pad	#16
	sub	sp, #16
	str	r0, [sp, #8]
	str	r1, [sp, #4]
	str	r2, [sp]
	ldr	r0, [sp, #8]
	cbnz	r0, .LBB5_2
	b	.LBB5_1
.LBB5_1:
	movs	r0, #1
	str	r0, [sp, #12]
	b	.LBB5_5
.LBB5_2:
	ldr	r0, [sp]
	ldr	r1, [sp, #4]
	mvns	r1, r1
	tst	r0, r1
	beq	.LBB5_4
	b	.LBB5_3
.LBB5_3:
	movs	r0, #2
	str	r0, [sp, #12]
	b	.LBB5_5
.LBB5_4:
	ldr	r0, [sp]
	ldr	r1, [sp, #8]
	str	r0, [r1]
	movs	r0, #0
	str	r0, [sp, #12]
	b	.LBB5_5
.LBB5_5:
	ldr	r0, [sp, #12]
	add	sp, #16
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
	.save	{r7, lr}
	push	{r7, lr}
	.pad	#32
	sub	sp, #32
	str	r0, [sp, #24]
	str	r1, [sp, #20]
	str	r2, [sp, #16]
	str	r3, [sp, #12]
	ldr	r0, [sp, #24]
	cbz	r0, .LBB6_3
	b	.LBB6_1
.LBB6_1:
	ldr	r0, [sp, #20]
	cbz	r0, .LBB6_3
	b	.LBB6_2
.LBB6_2:
	ldr	r0, [sp, #12]
	cbnz	r0, .LBB6_4
	b	.LBB6_3
.LBB6_3:
	movs	r0, #1
	str	r0, [sp, #28]
	b	.LBB6_5
.LBB6_4:
	ldr	r0, [sp, #24]
	ldr	r0, [r0]
	str	r0, [sp, #8]
	ldr	r1, [sp, #20]
	ldr	r0, [sp, #16]
	blx	r1
	ldr	r0, [sp, #24]
	ldr	r0, [r0]
	str	r0, [sp, #4]
	ldr	r0, [sp, #8]
	ldr	r1, [sp, #12]
	str	r0, [r1]
	ldr	r0, [sp, #4]
	ldr	r1, [sp, #12]
	str	r0, [r1, #4]
	movs	r0, #0
	str	r0, [sp, #28]
	b	.LBB6_5
.LBB6_5:
	ldr	r0, [sp, #28]
	add	sp, #32
	pop	{r7, pc}
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
	.save	{r7, lr}
	push	{r7, lr}
	.pad	#32
	sub	sp, #32
	str	r0, [sp, #24]
	str	r1, [sp, #20]
	str	r2, [sp, #16]
	str	r3, [sp, #12]
	ldr	r0, [sp, #24]
	cbz	r0, .LBB7_3
	b	.LBB7_1
.LBB7_1:
	ldr	r0, [sp, #20]
	cbz	r0, .LBB7_3
	b	.LBB7_2
.LBB7_2:
	ldr	r0, [sp, #12]
	cbnz	r0, .LBB7_4
	b	.LBB7_3
.LBB7_3:
	movs	r0, #1
	str	r0, [sp, #28]
	b	.LBB7_5
.LBB7_4:
	ldr	r0, [sp, #24]
	ldr	r0, [r0]
	str	r0, [sp, #8]
	ldr	r1, [sp, #20]
	ldr	r0, [sp, #16]
	blx	r1
	ldr	r0, [sp, #24]
	ldr	r0, [r0]
	str	r0, [sp, #4]
	ldr	r0, [sp, #8]
	ldr	r1, [sp, #12]
	str	r0, [r1]
	ldr	r0, [sp, #4]
	ldr	r1, [sp, #12]
	str	r0, [r1, #4]
	movs	r0, #0
	str	r0, [sp, #28]
	b	.LBB7_5
.LBB7_5:
	ldr	r0, [sp, #28]
	add	sp, #32
	pop	{r7, pc}
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
	.pad	#24
	sub	sp, #24
                                        @ kill: def $r12 killed $r1
                                        @ kill: def $r12 killed $r0
	str	r1, [sp, #16]
	str	r0, [sp, #12]
	str	r2, [sp, #8]
	str	r3, [sp, #4]
	ldr	r0, [sp, #12]
	cbnz	r0, .LBB8_2
	b	.LBB8_1
.LBB8_1:
	movs	r0, #1
	str	r0, [sp, #20]
	b	.LBB8_5
.LBB8_2:
	ldr	r0, [sp, #8]
	ldr	r1, [sp, #4]
	orrs	r0, r1
	ldr	r1, [sp, #16]
	mvns	r1, r1
	tst	r0, r1
	beq	.LBB8_4
	b	.LBB8_3
.LBB8_3:
	movs	r0, #2
	str	r0, [sp, #20]
	b	.LBB8_5
.LBB8_4:
	ldr	r0, [sp, #12]
	ldr	r0, [r0]
	str	r0, [sp]
	ldr	r0, [sp]
	ldr	r1, [sp, #8]
	bics	r0, r1
	ldr	r1, [sp, #4]
	orrs	r0, r1
	ldr	r1, [sp, #12]
	str	r0, [r1]
	movs	r0, #0
	str	r0, [sp, #20]
	b	.LBB8_5
.LBB8_5:
	ldr	r0, [sp, #20]
	add	sp, #24
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
	.pad	#16
	sub	sp, #16
                                        @ kill: def $r3 killed $r1
                                        @ kill: def $r3 killed $r0
	str	r1, [sp, #8]
	str	r0, [sp, #4]
	str	r2, [sp]
	ldr	r0, [sp, #4]
	cbnz	r0, .LBB9_2
	b	.LBB9_1
.LBB9_1:
	movs	r0, #1
	str	r0, [sp, #12]
	b	.LBB9_5
.LBB9_2:
	ldr	r0, [sp]
	ldr	r1, [sp, #8]
	mvns	r1, r1
	tst	r0, r1
	beq	.LBB9_4
	b	.LBB9_3
.LBB9_3:
	movs	r0, #2
	str	r0, [sp, #12]
	b	.LBB9_5
.LBB9_4:
	ldr	r0, [sp]
	ldr	r1, [sp, #4]
	str	r0, [r1]
	movs	r0, #0
	str	r0, [sp, #12]
	b	.LBB9_5
.LBB9_5:
	ldr	r0, [sp, #12]
	add	sp, #16
	bx	lr
.Lfunc_end9:
	.size	mpv_ack, .Lfunc_end9-mpv_ack
	.cantunwind
	.fnend
                                        @ -- End function
	.ident	"Ubuntu clang version 21.1.8 (6ubuntu1)"
	.section	".note.GNU-stack","",%progbits
	.addrsig
	.eabi_attribute	30, 6	@ Tag_ABI_optimization_goals
