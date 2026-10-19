.text
.globl aw_lower
.type aw_lower,@function
aw_lower:
 xorl %eax,%eax
 movl %ecx,%r10d
.Law_loop:
 cmpl %esi,%eax
 jae .Law_done
 movl %esi,%ecx
 subl %eax,%ecx
 shrl $1,%ecx
 addl %eax,%ecx
 movl 4(%rdi,%rcx,8),%r8d
 cmpl 4(%rdx),%r8d
 je .Law_id
 testl %r10d,%r10d
 jnz .Law_desc
 cmpl 4(%rdx),%r8d
 jl .Law_less
 jmp .Law_high
.Law_desc:
 cmpl 4(%rdx),%r8d
 jg .Law_less
 jmp .Law_high
.Law_id:
 movl (%rdi,%rcx,8),%r8d
 cmpl (%rdx),%r8d
 jl .Law_less
.Law_high:
 movl %ecx,%esi
 jmp .Law_loop
.Law_less:
 leal 1(%rcx),%eax
 jmp .Law_loop
.Law_done:
 ret
.size aw_lower,.-aw_lower
.section .note.GNU-stack,"",@progbits
