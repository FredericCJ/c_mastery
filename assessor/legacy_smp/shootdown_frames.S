.text
.code64
.global stg_bsp_enter
stg_bsp_enter:
 mov %rdi,%rsp
 xor %ebp,%ebp
 call stg_bsp
1: cli
 hlt
 jmp 1b
.global stg_irq
stg_irq:
 pushq $0
 pushq $246
 jmp stg_irq_common
.global stg_fault
stg_fault:
 pushq $14
stg_irq_common:
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
 call stg_interrupt
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
.global stg_try_write
.global stg_write_site
.global stg_after_write
stg_try_write:
 xor %eax,%eax
stg_write_site:
 mov %rsi,(%rdi)
 mov $1,%eax
stg_after_write:
 ret
.section .note.GNU-stack,"",@progbits
