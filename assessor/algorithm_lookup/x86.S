.text
.globl al_lookup_kernel
.type al_lookup_kernel,@function
al_lookup_kernel:
    movl $-1,(%rcx)
    movl $0,4(%rcx)
    movl 4(%rdx),%r8d
    movl (%rdx),%r9d
    xorl %r10d,%r10d
.Lvisit:
    movl %r8d,8(%rcx,%r10,4)
    incl %r10d
    movl %r10d,4(%rcx)
    leal (%r8,%r8,2),%eax
    leaq (%rdi,%rax,4),%rdx
    movl 8(%rdx),%eax
    testl %eax,%eax
    jz .Ldone
#ifdef AL_MUTANT_TOMBSTONE
    cmpl $2,%eax
    je .Ldone
#endif
    cmpl $1,%eax
    jne .Lnext
    cmpl %r9d,(%rdx)
    je .Lfound
.Lnext:
    cmpl %esi,%r10d
    jae .Ldone
    incl %r8d
    cmpl %esi,%r8d
    jne .Lvisit
    xorl %r8d,%r8d
    jmp .Lvisit
.Lfound:
    movl %r8d,(%rcx)
.Ldone:
    ret
.size al_lookup_kernel,.-al_lookup_kernel
.section .note.GNU-stack,"",@progbits
