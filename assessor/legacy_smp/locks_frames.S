.text
.code64
.global sl_bsp_enter
sl_bsp_enter:
 mov %rdi,%rsp
 xor %ebp,%ebp
 call sl_bsp
1: cli
 hlt
 jmp 1b
.macro SL_ISR name,number
.global \name
\name:
 pushq $0
 pushq $\number
 jmp sl_irq_common
.endm
SL_ISR sl_probe_irq,244
sl_irq_common:
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
 call sl_interrupt
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
