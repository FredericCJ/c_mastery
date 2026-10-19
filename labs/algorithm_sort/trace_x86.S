/* Called only after the instrumented kernel has aligned its call site. */
.text
.globl as_trace_entry
.type as_trace_entry,@function
as_trace_entry:
 pushq %rax
 pushq %rcx
 pushq %rdx
 pushq %rsi
 pushq %rdi
 pushq %r8
 pushq %r9
 pushq %r10
 pushq %r11
 leal 1(%r8),%edx
 call as_prefix
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
.size as_trace_entry,.-as_trace_entry
.section .note.GNU-stack,"",@progbits
