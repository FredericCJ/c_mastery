.text
.globl ab_registers
ab_registers:
 ret
.globl ab_dependency
ab_dependency:
 movq %rsi,%rax
 ret
.section .note.GNU-stack,"",@progbits
