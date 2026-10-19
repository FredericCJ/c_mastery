.text
.code64
.global swg_bsp_enter
swg_bsp_enter:
 /* Bootstrap scaffold only; queue/context policy remains unsolved. */
 mov %rdi,%rsp
 xor %ebp,%ebp
 call swg_bsp
1: cli
 hlt
 jmp 1b
.section .note.GNU-stack,"",@progbits
