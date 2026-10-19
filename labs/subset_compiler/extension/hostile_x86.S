.text
.globl ce_checked
ce_checked:
pushq %rbx
pushq %rbp
pushq %r12
pushq %r13
pushq %r14
pushq %r15
subq $8,%rsp
movq %rdx,(%rsp)
movq $4656,%rbx
movq $4657,%rbp
movq $4658,%r12
movq $4659,%r13
movq $4660,%r14
movq $4661,%r15
call cm_ext
cmpq $4656,%rbx
jne .Lbadabi
cmpq $4657,%rbp
jne .Lbadabi
cmpq $4658,%r12
jne .Lbadabi
cmpq $4659,%r13
jne .Lbadabi
cmpq $4660,%r14
jne .Lbadabi
cmpq $4661,%r15
jne .Lbadabi
jmp .Ldoneabi
.Lbadabi:
movq (%rsp),%rdx
movl $1,(%rdx)
.Ldoneabi:
addq $8,%rsp
popq %r15
popq %r14
popq %r13
popq %r12
popq %rbp
popq %rbx
ret
.section .note.GNU-stack,"",@progbits
