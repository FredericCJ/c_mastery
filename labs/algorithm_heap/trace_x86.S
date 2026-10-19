/* Called only after the instrumented kernel has aligned its call site. */
.text
.globl ah_trace_entry
.type ah_trace_entry,@function
ah_trace_entry:
 pushq %rax
 pushq %rcx
 pushq %rdx
 pushq %rsi
 pushq %rdi
 pushq %r8
 pushq %r9
 pushq %r10
 pushq %r11
 movl %r8d,%edx
 call ah_snapshot
 popq %r11
 popq %r10
 popq %r9
 popq %r8
 popq %rdi
 popq %rsi
 popq %rdx
 popq %rcx
 popq %rax
 ret
.size ah_trace_entry,.-ah_trace_entry
.section .note.GNU-stack,"",@progbits
