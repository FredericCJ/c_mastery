.text
.code64
.global swg_bsp_enter
swg_bsp_enter:
 mov %rdi,%rsp
 xor %ebp,%ebp
 call swg_bsp
1: cli
 hlt
 jmp 1b
.macro SL_ISR name,number
.global \name
\name:
 pushq $0
 pushq $\number
 jmp swg_irq_common
.endm
SL_ISR swg_irq,245
swg_irq_common:
 cld
 push %rax
 push %rbx
 push %rcx
 push %rdx
 push %rbp
 push %rsi
 push %rdi
 push %r8
 push %r9
 push %r10
 push %r11
 push %r12
 push %r13
 push %r14
 push %r15
 mov %rsp,%r12
 mov %rsp,%rdi
 and $-16,%rsp
 call swg_interrupt
 mov %r12,%rsp
 pop %r15
 pop %r14
 pop %r13
 pop %r12
 pop %r11
 pop %r10
 pop %r9
 pop %r8
 pop %rdi
 pop %rsi
 pop %rbp
 pop %rdx
 pop %rcx
 pop %rbx
 pop %rax
 add $16,%rsp
 iretq

.text
.global swg_sleep
.global swg_after_halt
swg_sleep:
 sti
 hlt
swg_after_halt:
 cli
 ret
.global swg_bad_sleep
.global swg_after_bad_halt
swg_bad_sleep:
 hlt
swg_after_bad_halt:
 cli
 ret
