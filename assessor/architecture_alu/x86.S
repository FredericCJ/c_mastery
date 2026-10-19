.text
.global aa_isa_compare
.type aa_isa_compare,@function
aa_isa_compare:
 cmpl %esi,%edi
 pushfq
 popq %r8
 movl $0,%edx
 jae 1f
 orl $1,%edx
1: cmpl %esi,%edi
 jbe 2f
 orl $2,%edx
2: cmpl %esi,%edi
 jge 3f
 orl $4,%edx
3: cmpl %esi,%edi
 jle 4f
 orl $8,%edx
4: cmpl %esi,%edi
 jne 5f
 orl $16,%edx
5: movl %r8d,%eax
 andl $1,%eax
 shll $5,%eax
 orl %eax,%edx
 movl %r8d,%eax
 shrl $11,%eax
 andl $1,%eax
 shll $6,%eax
 orl %eax,%edx
 andl $128,%r8d
 orl %r8d,%edx
 movl %edx,%eax
 ret
.size aa_isa_compare,.-aa_isa_compare
.section .note.GNU-stack,"",@progbits
