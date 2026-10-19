.text
.globl al_lookup_abi
.type al_lookup_abi,@function
al_lookup_abi:
    pushq %rbx
    pushq %rbp
    pushq %r12
    pushq %r13
    pushq %r14
    pushq %r15
    subq $8,%rsp
    movq %rsp,(%rsp)
    movl $0x1122,%ebx
    movl $0x2233,%ebp
    movl $0x3344,%r12d
    movl $0x4455,%r13d
    movl $0x5566,%r14d
    movl $0x6677,%r15d
    call al_lookup_kernel
    xorl %eax,%eax
    cmpq %rsp,(%rsp)
    jne .Lend
    cmpq $0x1122,%rbx
    jne .Lend
    cmpq $0x2233,%rbp
    jne .Lend
    cmpq $0x3344,%r12
    jne .Lend
    cmpq $0x4455,%r13
    jne .Lend
    cmpq $0x5566,%r14
    jne .Lend
    cmpq $0x6677,%r15
    jne .Lend
    movl $1,%eax
.Lend:
    addq $8,%rsp
    popq %r15
    popq %r14
    popq %r13
    popq %r12
    popq %rbp
    popq %rbx
    ret
.size al_lookup_abi,.-al_lookup_abi
.section .note.GNU-stack,"",@progbits
