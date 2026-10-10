.text
.globl mi_hostile
.type mi_hostile,@function
mi_hostile:
 pushq %rbx
 pushq %rbp
 pushq %r12
 pushq %r13
 pushq %r14
 pushq %r15
 subq $8,%rsp
 movq $0x1234567,%rbx
 movq $0x2345678,%rbp
 movq $0x3456789,%r12
 movq $0x456789a,%r13
 movq $0x56789ab,%r14
 movq $0x6789abc,%r15
 call mi_asm_sum
 cmpq $0x1234567,%rbx
 jne 1f
 cmpq $0x2345678,%rbp
 jne 1f
 cmpq $0x3456789,%r12
 jne 1f
 cmpq $0x456789a,%r13
 jne 1f
 cmpq $0x56789ab,%r14
 jne 1f
 cmpq $0x6789abc,%r15
 jne 1f
 jmp 2f
1: movq $-1,%rax
2: addq $8,%rsp
 popq %r15
 popq %r14
 popq %r13
 popq %r12
 popq %rbp
 popq %rbx
 ret
.size mi_hostile,.-mi_hostile
.section .note.GNU-stack,"",@progbits
