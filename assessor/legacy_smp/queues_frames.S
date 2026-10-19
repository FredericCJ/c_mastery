.text
.code64
.global sqg_bsp_enter
sqg_bsp_enter:
 mov %rdi,%rsp
 xor %ebp,%ebp
 call sqg_bsp
1: cli
 hlt
 jmp 1b
.section .note.GNU-stack,"",@progbits
