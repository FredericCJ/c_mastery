.text
.code64
.global sbg_bsp_enter
sbg_bsp_enter:
 mov %rdi,%rsp
 xor %ebp,%ebp
 call sbg_bsp
1: cli
 hlt
 jmp 1b
.section .note.GNU-stack,"",@progbits
