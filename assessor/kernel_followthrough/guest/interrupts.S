.text
.code64
.macro NOERR n
.global isr\n
isr\n:
 pushq $0
 pushq $\n
 jmp common_entry
.endm
.macro ERR n
.global isr\n
isr\n:
 pushq $\n
 jmp common_entry
.endm
NOERR 0
NOERR 1
NOERR 2
NOERR 3
NOERR 4
NOERR 5
NOERR 6
NOERR 7
ERR 8
NOERR 9
ERR 10
ERR 11
ERR 12
ERR 13
ERR 14
NOERR 15
NOERR 16
ERR 17
NOERR 18
NOERR 19
NOERR 20
ERR 21
NOERR 22
NOERR 23
NOERR 24
NOERR 25
NOERR 26
NOERR 27
NOERR 28
ERR 29
ERR 30
NOERR 31
NOERR 32
NOERR 128
common_entry:
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
 mov %rsp,%rdi
#if KF_CASE == 16
 pushq $0x55 /* extra diagnostic word below the unchanged176-byte frame */
#endif
 and $-16,%rsp
 call dispatch
 mov %rax,%rsp
.global restore_user
restore_user:
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
.global enter_frame
enter_frame:
 mov %rdi,%rsp
 jmp restore_user
.section .rodata
.global isr_table
isr_table:
.quad isr0,isr1,isr2,isr3,isr4,isr5,isr6,isr7,isr8,isr9,isr10,isr11,isr12,isr13,isr14,isr15
.quad isr16,isr17,isr18,isr19,isr20,isr21,isr22,isr23,isr24,isr25,isr26,isr27,isr28,isr29,isr30,isr31
.section .note.GNU-stack,"",@progbits
