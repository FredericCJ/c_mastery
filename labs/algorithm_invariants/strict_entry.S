.text
.globl _start
.type _start,@function
_start:
    andq $-16,%rsp
    call ai_strict
    movl %eax,%edi
    movl $60,%eax
    syscall
    ud2
.size _start,.-_start
.section .note.GNU-stack,"",@progbits
