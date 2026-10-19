.text
.globl ab_ms_lower
.type ab_ms_lower,@function
ab_ms_lower:
    /* Microsoft x64 home slots belong to this callee; no RSP/nonvolatile change. */
    movq %rcx,8(%rsp)
    movq %rdx,16(%rsp)
    movq %r8,24(%rsp)
    xorl %eax,%eax
    movl %edx,%r9d
.Lms_loop:
    cmpl %r9d,%eax
    jae .Lms_done
    movl %r9d,%r10d
    subl %eax,%r10d
    shrl $1,%r10d
    addl %eax,%r10d
    cmpl %r8d,(%rcx,%r10,4)
    jb .Lms_right
    movl %r10d,%r9d
    jmp .Lms_loop
.Lms_right:
    leal 1(%r10),%eax
    jmp .Lms_loop
.Lms_done:
    ret
.size ab_ms_lower,.-ab_ms_lower

.globl ab_records_lower
.type ab_records_lower,@function
ab_records_lower:
    xorl %eax,%eax
    movq %rsi,%rcx
.Lrecord_loop:
    cmpq %rcx,%rax
    jae .Lrecord_done
    movq %rcx,%r8
    subq %rax,%r8
    shrq $1,%r8
    addq %rax,%r8
    cmpl %edx,(%rdi,%r8,8)
    jb .Lrecord_right
    movq %r8,%rcx
    jmp .Lrecord_loop
.Lrecord_right:
    leaq 1(%r8),%rax
    jmp .Lrecord_loop
.Lrecord_done:
    ret
.size ab_records_lower,.-ab_records_lower

.globl ab_callback_lower
.type ab_callback_lower,@function
ab_callback_lower:
    pushq %rbx
    pushq %rbp
    pushq %r12
    pushq %r13
    pushq %r14
    pushq %r15
    subq $24,%rsp
    movq %rdi,%rbx
    movl %esi,%r13d
    xorl %r14d,%r14d
    movl %edx,%r15d
    movq %rcx,%rbp
    movq %r8,0(%rsp)
.Lcallback_loop:
    cmpl %r13d,%r14d
    jae .Lcallback_done
    movl %r13d,%r12d
    subl %r14d,%r12d
    shrl $1,%r12d
    addl %r14d,%r12d
    movq 0(%rsp),%rdi
    movq %rbx,%rsi
    movl %r12d,%edx
    call *%rbp
    cmpl %r15d,%eax
    jb .Lcallback_right
    movl %r12d,%r13d
    jmp .Lcallback_loop
.Lcallback_right:
    leal 1(%r12),%r14d
    jmp .Lcallback_loop
.Lcallback_done:
    movl %r14d,%eax
#ifdef AB_MUTANT_CALLBACK
    movl %r10d,%eax
#endif
    addq $24,%rsp
    popq %r15
    popq %r14
    popq %r13
    popq %r12
    popq %rbp
    popq %rbx
    ret
.size ab_callback_lower,.-ab_callback_lower

.section .note.GNU-stack,"",@progbits
