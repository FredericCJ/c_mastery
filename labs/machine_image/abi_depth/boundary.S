/* Deliberately incomplete learner boundary with a valid object interface. */
.data
.globl ad_entry_mod,ad_before8,ad_entry8,ad_seventh,ad_eighth
ad_entry_mod: .quad 0
ad_before8: .quad 0
ad_entry8: .quad 0
ad_seventh: .quad 0
ad_eighth: .quad 0
.text
.globl ad_sum_entry,ad_call8,ad_cfi
.type ad_sum_entry,@function
ad_sum_entry:
 xorq %rax,%rax
 ret
.size ad_sum_entry,.-ad_sum_entry
.type ad_call8,@function
ad_call8:
 xorq %rax,%rax
 ret
.size ad_call8,.-ad_call8
.type ad_cfi,@function
ad_cfi:
 ret
.size ad_cfi,.-ad_cfi
.section .note.GNU-stack,"",@progbits
