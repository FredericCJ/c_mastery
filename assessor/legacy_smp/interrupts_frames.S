.text
.code64
.global si_bsp_enter
si_bsp_enter:
 mov %rdi,%rsp
 xor %ebp,%ebp
 call si_bsp
1: cli
 hlt
 jmp 1b
.macro SI_ISR name,number
.global \name
\name:
 pushq $0
 pushq $\number
 jmp si_irq_common
.endm
SI_ISR si_timer_irq,240
SI_ISR si_ipi_irq,241
SI_ISR si_spurious_irq,255
si_irq_common:
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
 call si_interrupt
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
