/* Trusted test harness. Learners implement the callable boundary separately. */
.text
.globl ad_hostile
.type ad_hostile,@function
ad_hostile:
 pushq %rbx
 pushq %rbp
 pushq %r12
 pushq %r13
 pushq %r14
 pushq %r15
 subq $40,%rsp
 movq %rcx,0(%rsp)
 movq %rdx,8(%rsp)
 movq %rsp,%rax
 andq $15,%rax
 movq %rax,8(%rcx)
 movq 0(%rdx),%rbx
 movq 8(%rdx),%rbp
 movq 16(%rdx),%r12
 movq 24(%rdx),%r13
 movq 32(%rdx),%r14
 movq 40(%rdx),%r15
 call ad_sum_entry
 movq 0(%rsp),%rcx
 movq %rax,0(%rcx)
 movq ad_entry_mod(%rip),%rax
 movq %rax,16(%rcx)
 movq 8(%rsp),%r10
 xorq %r11,%r11
 cmpq 0(%r10),%rbx
 je 1f
 orq $1,%r11
1: cmpq 8(%r10),%rbp
 je 2f
 orq $2,%r11
2: cmpq 16(%r10),%r12
 je 3f
 orq $4,%r11
3: cmpq 24(%r10),%r13
 je 4f
 orq $8,%r11
4: cmpq 32(%r10),%r14
 je 5f
 orq $16,%r11
5: cmpq 40(%r10),%r15
 je 6f
 orq $32,%r11
6: movq %r11,32(%rcx)
 testq %r11,%r11
 sete %al
 movzbq %al,%rax
 movq %rax,24(%rcx)
 addq $40,%rsp
 popq %r15
 popq %r14
 popq %r13
 popq %r12
 popq %rbp
 popq %rbx
 ret
.size ad_hostile,.-ad_hostile
.section .note.GNU-stack,"",@progbits
