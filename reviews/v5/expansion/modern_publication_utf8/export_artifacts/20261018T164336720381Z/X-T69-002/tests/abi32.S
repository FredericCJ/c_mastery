.text
.globl _start
.type _start,@function
_start:
 xorl %ebp,%ebp
 andl $-16,%esp
 call abi_main
 movl %eax,%ebx
 movl $1,%eax
 int $0x80
.size _start,.-_start
.globl abi_write
.type abi_write,@function
abi_write:
 pushl %ebx
 movl 8(%esp),%ecx
 movl 12(%esp),%edx
 movl $1,%ebx
 movl $4,%eax
 int $0x80
 popl %ebx
 ret
.size abi_write,.-abi_write
.section .note.GNU-stack,"",@progbits
