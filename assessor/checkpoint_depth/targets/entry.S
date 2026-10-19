.text
.global _start
.type _start,@function
_start:
    mov %rsp,%rdi
    xor %ebp,%ebp
    and $-16,%rsp
    call cpw_image
    mov %eax,%edi
    mov $60,%eax
    syscall
    ud2
.size _start,.-_start
.section .note.GNU-stack,"",@progbits
