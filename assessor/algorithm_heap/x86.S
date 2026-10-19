.text
.globl ah_cd_sink
.type ah_cd_sink,@function
ah_cd_sink:
.Ltop:
    movl 448(%rdi),%ecx
    leal 1(%rsi,%rsi),%r8d
    cmpl %ecx,%r8d
    jae .Ldone
    leal 1(%r8),%r9d
    cmpl %ecx,%r9d
    jae .Lchosen
    incq 464(%rdi)
    movl %r8d,%eax
    shll $4,%eax
    movl %r9d,%ecx
    shll $4,%ecx
    movl (%rdi,%rcx),%edx
    cmpl (%rdi,%rax),%edx
    je .Lsibling_tie
    cmpl $0,452(%rdi)
    je .Lsibling_min
    cmpl (%rdi,%rax),%edx
    jg .Lright
    jmp .Lchosen
.Lsibling_min:
    cmpl (%rdi,%rax),%edx
    jl .Lright
    jmp .Lchosen
.Lsibling_tie:
    movl 12(%rdi,%rcx),%edx
    cmpl 12(%rdi,%rax),%edx
    jae .Lchosen
.Lright:
    movl %r9d,%r8d
.Lchosen:
    incq 464(%rdi)
    movl %esi,%eax
    shll $4,%eax
    movl %r8d,%ecx
    shll $4,%ecx
    movl (%rdi,%rcx),%edx
    cmpl (%rdi,%rax),%edx
    je .Lparent_tie
    cmpl $0,452(%rdi)
    je .Lparent_min
    cmpl (%rdi,%rax),%edx
    jg .Lswap
    jmp .Ldone
.Lparent_min:
    cmpl (%rdi,%rax),%edx
    jl .Lswap
    jmp .Ldone
.Lparent_tie:
    movl 12(%rdi,%rcx),%edx
    cmpl 12(%rdi,%rax),%edx
    jae .Ldone
.Lswap:
    movq (%rdi,%rax),%rdx
    movq 8(%rdi,%rax),%r9
    movq (%rdi,%rcx),%r10
    movq 8(%rdi,%rcx),%r11
    movq %r10,(%rdi,%rax)
    movq %r11,8(%rdi,%rax)
    movq %rdx,(%rdi,%rcx)
    movq %r9,8(%rdi,%rcx)
#ifndef AH_MUTANT_INDEX
    movl %r11d,%edx
    movl %esi,256(%rdi,%rdx,4)
    movl %r9d,%edx
    movl %r8d,256(%rdi,%rdx,4)
#endif
    movl %r8d,%esi
    jmp .Ltop
.Ldone:
    ret
.size ah_cd_sink,.-ah_cd_sink
.section .note.GNU-stack,"",@progbits
