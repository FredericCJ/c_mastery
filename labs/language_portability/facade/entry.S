.text
.global _start
.type _start,@function
_start:
 xor %rbp,%rbp
 and $-16,%rsp
 call bf_target_main
 mov %eax,%edi
 mov $60,%eax
 syscall
 ud2
.size _start,.-_start
.section .note.GNU-stack,"",@progbits
