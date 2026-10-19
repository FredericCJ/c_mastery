.text
.globl ab_poison
.type ab_poison,@function
ab_poison:
    addl $1,0(%rdi)
    movq %rsp,%rax
    andl $15,%eax
    cmpl $8,%eax
    je .Laligned
    addl $1,4(%rdi)
.Laligned:
    movl 8(%rdi),%ecx
    movl (%rsi,%rdx,4),%eax
    movl %ecx,%edi
    movl %ecx,%esi
    movl %ecx,%edx
    movl %ecx,%r8d
    movl %ecx,%r9d
    movl %ecx,%r10d
    movl %ecx,%r11d
    ret
.size ab_poison,.-ab_poison
.section .note.GNU-stack,"",@progbits
