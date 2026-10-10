.text
.globl _start
.type _start,@function
_start:
 movq %rsp,%rdi
 andq $-16,%rsp
 call mi_entry
 movq %rax,%rdi
 movq $60,%rax
 syscall
.size _start,.-_start
.section .note.GNU-stack,"",@progbits
