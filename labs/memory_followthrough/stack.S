.text
.globl mw_stack_probe
.type mw_stack_probe,@function
/* SysV x86-64, no calls/red-zone dependence. Reserve64 actual stack bytes,
   paint all64, touch32, restore8 touched bytes, sample minimum stack pointer.
   Result: changed24, reservation64. Caller-owned result remains outside frame. */
mw_stack_probe:
 mov %rsp,%r8
 sub $64,%rsp
 xor %ecx,%ecx
1: movb $0xa5,(%rsp,%rcx)
 inc %rcx
 cmp $64,%rcx
 jne 1b
 xor %ecx,%ecx
2: movb $0,(%rsp,%rcx)
 inc %rcx
 cmp $32,%rcx
 jne 2b
 xor %ecx,%ecx
3: movb $0xa5,(%rsp,%rcx)
 inc %rcx
 cmp $8,%rcx
 jne 3b
 xor %ecx,%ecx
 xor %eax,%eax
4: cmpb $0xa5,(%rsp,%rcx)
 je 5f
 inc %rax
5: inc %rcx
 cmp $64,%rcx
 jne 4b
 mov %rax,(%rdi)
 sub %rsp,%r8
 mov %r8,8(%rdi)
 add $64,%rsp
 ret
.size mw_stack_probe,.-mw_stack_probe
.section .note.GNU-stack,"",@progbits
