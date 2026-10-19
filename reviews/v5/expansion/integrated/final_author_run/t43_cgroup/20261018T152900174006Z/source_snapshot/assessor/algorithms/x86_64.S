.text
.globl algo_asm_lower
.type algo_asm_lower,@function
algo_asm_lower:
    xorl %eax,%eax
    movl %esi,%ecx
.Llower_loop:
    cmpl %ecx,%eax
    jae .Llower_done
    movl %ecx,%r8d
    subl %eax,%r8d
    shrl $1,%r8d
    addl %eax,%r8d
    cmpl %edx,(%rdi,%r8,4)
#ifdef ALGO_ASM_SIGNED
    jl .Llower_right
#else
    jb .Llower_right
#endif
    movl %r8d,%ecx
    jmp .Llower_loop
.Llower_right:
    leal 1(%r8),%eax
    jmp .Llower_loop
.Llower_done:
#ifdef ALGO_ASM_ABI
    xorl %ebx,%ebx
#endif
    ret
.size algo_asm_lower,.-algo_asm_lower

.globl algo_asm_insert
.type algo_asm_insert,@function
algo_asm_insert:
    movl $1,%r8d
.Linsert_outer:
    cmpl %esi,%r8d
    jae .Linsert_done
    movq (%rdi,%r8,8),%r10
    movl %r8d,%r9d
.Linsert_inner:
    testl %r9d,%r9d
    jz .Linsert_place
    movq -8(%rdi,%r9,8),%r11
    cmpl %r10d,%r11d
#ifdef ALGO_ASM_UNSTABLE
    jb .Linsert_place
#else
    jbe .Linsert_place
#endif
    movq %r11,(%rdi,%r9,8)
    decl %r9d
    jmp .Linsert_inner
.Linsert_place:
    movq %r10,(%rdi,%r9,8)
    incl %r8d
    jmp .Linsert_outer
.Linsert_done:
    ret
.size algo_asm_insert,.-algo_asm_insert

.globl algo_asm_sift
.type algo_asm_sift,@function
algo_asm_sift:
    movl %edx,%r8d
.Lsift_loop:
    leal 1(%r8,%r8),%eax
    cmpl %esi,%eax
    jae .Lsift_done
    movl (%rdi,%rax,4),%r9d
    leal 1(%rax),%r10d
    cmpl %esi,%r10d
    jae .Lsift_chosen
    movl (%rdi,%r10,4),%r11d
    cmpl %r9d,%r11d
    jbe .Lsift_chosen
    movl %r10d,%eax
    movl %r11d,%r9d
.Lsift_chosen:
    movl (%rdi,%r8,4),%edx
    cmpl %r9d,%edx
    jae .Lsift_done
    movl %r9d,(%rdi,%r8,4)
    movl %edx,(%rdi,%rax,4)
    movl %eax,%r8d
    jmp .Lsift_loop
.Lsift_done:
    ret
.size algo_asm_sift,.-algo_asm_sift
.section .note.GNU-stack,"",@progbits
