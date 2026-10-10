.text
.globl mi_asm_sum
.type mi_asm_sum,@function
mi_asm_sum:
 xorl %eax,%eax
 testq %rsi,%rsi
 jz 2f
1: movl (%rdi),%edx
 addq %rdx,%rax
 addq $4,%rdi
 decq %rsi
 jnz 1b
2: ret
.size mi_asm_sum,.-mi_asm_sum
.section .note.GNU-stack,"",@progbits
