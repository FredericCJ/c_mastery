.text
.code64
.global stg_bsp_enter
stg_bsp_enter:
 mov %rdi,%rsp
 xor %ebp,%ebp
 call stg_bsp
1: cli
 hlt
 jmp 1b
.section .note.GNU-stack,"",@progbits
