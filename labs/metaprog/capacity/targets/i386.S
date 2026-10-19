.text
.global _start
.type _start,@function
_start:
 andl $-16,%esp
 call mpc32_test
 movl %eax,%ebx
 movl $1,%eax
 int $0x80
.global mpc32_write
.type mpc32_write,@function
mpc32_write:
 pushl %ebx
 movl 8(%esp),%ecx
 movl 12(%esp),%edx
 movl $1,%ebx
 movl $4,%eax
 int $0x80
 popl %ebx
 ret
.section .note.GNU-stack,"",@progbits
