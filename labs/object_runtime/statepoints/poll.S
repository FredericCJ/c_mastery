.text
.globl gc_poll
.type gc_poll,@function
gc_poll:
  leaq 8(%rsp),%rdi
  movq (%rsp),%rsi
  subq $8,%rsp
  call relocate_roots
  addq $8,%rsp
  ret
.size gc_poll,.-gc_poll
.section .note.GNU-stack,"",@progbits
