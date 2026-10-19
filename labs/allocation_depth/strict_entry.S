.text
.global _start
_start:
 and $-16,%rsp
 call al_image
 mov %eax,%edi
 mov $60,%eax
 syscall
.section .note.GNU-stack,"",@progbits
