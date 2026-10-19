.syntax unified
.thumb
.global df_memmanage
.type df_memmanage,%function
.thumb_func
df_memmanage:
 tst lr,#4
 ite eq
 mrseq r0,msp
 mrsne r0,psp
 mov r1,lr
 b df_handler
.size df_memmanage,.-df_memmanage
.global df_store
.type df_store,%function
.thumb_func
df_store:
.global df_fault_instruction
df_fault_instruction:
 str r0,[r1,#0]
.global df_fault_resume
df_fault_resume:
 bx lr
.size df_store,.-df_store
