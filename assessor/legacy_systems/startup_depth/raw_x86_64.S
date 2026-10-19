.text
.globl sd_raw6
.type sd_raw6,@function
sd_raw6:
 mov %rdi,%rax
 mov %rsi,%rdi
 mov %rdx,%rsi
 mov %rcx,%rdx
 mov %r8,%r10
 mov %r9,%r8
 mov 8(%rsp),%r9
 syscall
 ret
.size sd_raw6,.-sd_raw6
.section .note.GNU-stack,"",@progbits
