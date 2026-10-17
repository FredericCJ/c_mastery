.text
.globl co_context_switch
.type co_context_switch,@function
co_context_switch:
 movq %rsp,0(%rdi)
 movq %rbx,8(%rdi)
 movq %rbp,16(%rdi)
 movq %r12,24(%rdi)
 movq %r13,32(%rdi)
 movq %r14,40(%rdi)
 movq %r15,48(%rdi)
 stmxcsr 56(%rdi)
 fnstcw 60(%rdi)
 movq 0(%rsi),%rsp
 movq 8(%rsi),%rbx
 movq 16(%rsi),%rbp
 movq 24(%rsi),%r12
 movq 32(%rsi),%r13
 movq 40(%rsi),%r14
 movq 48(%rsi),%r15
#ifndef CR_MUTATE_FIBER
 ldmxcsr 56(%rsi)
 fldcw 60(%rsi)
#endif
 ret
.size co_context_switch,.-co_context_switch
.globl co_context_entry
.type co_context_entry,@function
co_context_entry:
 subq $8,%rsp
 movq %r12,%rdi
 call co_fiber_boot
 ud2
.size co_context_entry,.-co_context_entry
.section .note.GNU-stack,"",@progbits
