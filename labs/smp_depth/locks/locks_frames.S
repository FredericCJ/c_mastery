.text
.code64
.global sl_bsp_enter
sl_bsp_enter:
 mov %rdi,%rsp
 xor %ebp,%ebp
 call sl_bsp
1: cli
 hlt
 jmp 1b
