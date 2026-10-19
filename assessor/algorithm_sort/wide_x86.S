.text
.globl as_wide_insert
.type as_wide_insert,@function
as_wide_insert:
 movl $1,%r8d
.Lwide_outer:
 cmpl %esi,%r8d
 jae .Lwide_done
 movl %r8d,%eax
 shlq $4,%rax
 movl (%rdi,%rax),%r10d
#ifdef AS_MUTANT_WIDE
 movl 8(%rdi,%rax),%r11d
#else
 movq 8(%rdi,%rax),%r11
#endif
 movl %r8d,%r9d
.Lwide_inner:
 testl %r9d,%r9d
 jz .Lwide_place
 movl -16(%rdi,%rax),%ecx
 cmpl %r10d,%ecx
 jbe .Lwide_place
 movq -8(%rdi,%rax),%rdx
 movl %ecx,(%rdi,%rax)
 movq %rdx,8(%rdi,%rax)
 decl %r9d
 subq $16,%rax
 jmp .Lwide_inner
.Lwide_place:
 movl %r10d,(%rdi,%rax)
 movq %r11,8(%rdi,%rax)
 incl %r8d
 jmp .Lwide_outer
.Lwide_done:
 ret
.size as_wide_insert,.-as_wide_insert
.section .note.GNU-stack,"",@progbits
