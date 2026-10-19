.text
.code64
.global srg_bsp_enter
srg_bsp_enter:
 mov %rdi,%rsp
 xor %ebp,%ebp
 call srg_bsp
1: cli
 hlt
 jmp 1b
.section .note.GNU-stack,"",@progbits
