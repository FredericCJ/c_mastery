.text
.globl ab_registers
.type ab_registers,@function
ab_registers:
 movabs $0x1122334455667788,%rax
 movb $0xaa,%al
 movq %rax,0(%rdi)
 movw $0xbbcc,%ax
 movq %rax,8(%rdi)
 movl $0x89abcdef,%eax
 movq %rax,16(%rdi)
 ret
.size ab_registers,.-ab_registers
.globl ab_dependency
.type ab_dependency,@function
ab_dependency:
 movq %rsi,%rax
 testq %rdx,%rdx
 jz .Ldone
 testl %edi,%edi
 jnz .Lfull
.p2align 4
.Lpartial:
 movb $17,%al
 addq %rcx,%rax
 andl $255,%eax
 decq %rdx
 jnz .Lpartial
 ret
.p2align 4
.Lfull:
 movl $17,%eax
 addq %rcx,%rax
 andl $255,%eax
 decq %rdx
 jnz .Lfull
.Ldone:
 ret
.size ab_dependency,.-ab_dependency
.section .note.GNU-stack,"",@progbits
