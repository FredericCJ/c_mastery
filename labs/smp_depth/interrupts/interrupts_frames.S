.text
.code64
.global si_bsp_enter
si_bsp_enter:
 mov %rdi,%rsp
 xor %ebp,%ebp
 call si_bsp
1: cli
 hlt
 jmp 1b
