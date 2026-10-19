.text
.globl as_wide_abi_probe
as_wide_abi_probe:
    leaq as_wide_insert(%rip),%r11
    jmp .Lprobe
.globl as_wide_abi_insert_probe
as_wide_abi_insert_probe:
    leaq algo_asm_insert(%rip),%r11
    jmp .Lprobe
.globl as_wide_abi_sift_probe
as_wide_abi_sift_probe:
    leaq algo_asm_sift(%rip),%r11
.Lprobe:
    pushq %rbx
    pushq %rbp
    pushq %r12
    pushq %r13
    pushq %r14
    pushq %r15
    subq $40,%rsp
    movq %rcx,0(%rsp)
    movq %r11,8(%rsp)
    movq %rsp,16(%rsp)
    movl $0x1122,%ebx
    movl $0x2233,%ebp
    movl $0x3344,%r12d
    movl $0x4455,%r13d
    movl $0x5566,%r14d
    movl $0x6677,%r15d
    call .Lnested
    movq 0(%rsp),%r10
    movl %eax,(%r10)
    xorl %eax,%eax
    cmpq %rsp,16(%rsp)
    jne .Lrestore
    cmpq $0x1122,%rbx
    jne .Lrestore
    cmpq $0x2233,%rbp
    jne .Lrestore
    cmpq $0x3344,%r12
    jne .Lrestore
    cmpq $0x4455,%r13
    jne .Lrestore
    cmpq $0x5566,%r14
    jne .Lrestore
    cmpq $0x6677,%r15
    jne .Lrestore
    movl $1,%eax
.Lrestore:
    addq $40,%rsp
    popq %r15
    popq %r14
    popq %r13
    popq %r12
    popq %rbp
    popq %rbx
    ret
.Lnested:
    movq %rsp,%rax
    andl $15,%eax
    cmpl $8,%eax
    jne .Lbad_alignment
    movl $0xdeadbeef,%r8d
    movl $0xabcdef01,%r9d
    movl $0x12345678,%r10d
    movl $0x23456789,%ecx
    subq $8,%rsp
    call *%r11
    addq $8,%rsp
    ret
.Lbad_alignment:
    xorl %ebx,%ebx
    ret
.section .note.GNU-stack,"",@progbits
