.syntax unified
.cpu cortex-m4
.thumb
.section .vectors,"a",%progbits
.global cf_vectors
cf_vectors:
 .word __stack_top
#ifdef CF_WRONG_VECTOR
 .word 0
#else
 .word cf_reset
#endif
 .word cf_trap,cf_trap,cf_memmanage,cf_trap,cf_trap,0,0,0,0,cf_svc,cf_trap,0,cf_pendsv,cf_trap
.section .text.cf_reset,"ax",%progbits
.global cf_reset
.type cf_reset,%function
.thumb_func
cf_reset:
 ldr r0,=__data_load
 ldr r1,=__data_start
 ldr r2,=__data_end
#ifdef CF_BAD_COPY
 subs r2,#4
#endif
1: cmp r1,r2
 bhs 2f
 ldr r3,[r0],#4
 str r3,[r1],#4
 b 1b
2:
#ifndef CF_NO_BSS
 ldr r1,=__bss_start
 ldr r2,=__bss_end
 movs r3,#0
3: cmp r1,r2
 bhs 4f
 str r3,[r1],#4
 b 3b
#endif
4: bl cf_target_main
 b cf_trap
.size cf_reset,.-cf_reset
.global cf_memmanage,cf_svc
.type cf_memmanage,%function
.thumb_func
cf_memmanage:
 tst lr,#4
 ite eq
 mrseq r0,msp
 mrsne r0,psp
 b cf_fault_handler
.type cf_svc,%function
.thumb_func
cf_svc:
 tst lr,#4
 ite eq
 mrseq r0,msp
 mrsne r0,psp
 b cf_svc_handler
.global cf_user_store,cf_denied_instruction,cf_denied_resume
.type cf_user_store,%function
.thumb_func
cf_user_store:
cf_denied_instruction:
 str r1,[r0]
cf_denied_resume:
 movs r0,#0
 movs r1,#8
 svc #0
 bx lr
.size cf_user_store,.-cf_user_store
