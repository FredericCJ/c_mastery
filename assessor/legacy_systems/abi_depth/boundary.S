.data
.globl ad_entry_mod,ad_before8,ad_entry8,ad_seventh,ad_eighth
ad_entry_mod: .quad 0
ad_before8: .quad 0
ad_entry8: .quad 0
ad_seventh: .quad 0
ad_eighth: .quad 0
.text
.globl ad_sum_entry
.type ad_sum_entry,@function
ad_sum_entry:
 movq %rsp,%rax
 andq $15,%rax
 movq %rax,ad_entry_mod(%rip)
 pushq %rbx
 subq $16,%rsp
 movq %rdi,%rbx
 movq %rsi,0(%rsp)
 call ad_scratch_call
 movq %rbx,%rdi
 movq 0(%rsp),%rsi
 call mi_asm_sum
 addq $16,%rsp
 popq %rbx
#ifdef AD_MUTANT_PRESERVED
 xorq $1,%rbx
#endif
 ret
.size ad_sum_entry,.-ad_sum_entry
.type ad_scratch_call,@function
ad_scratch_call:
 movq $91,%rdi
 movq $92,%rsi
 movq $93,%rdx
 movq $94,%rcx
 movq $95,%r8
 movq $96,%r9
 movq $97,%r10
 movq $98,%r11
 movq $99,%rax
 ret
.size ad_scratch_call,.-ad_scratch_call
.globl ad_call8
.type ad_call8,@function
ad_call8:
#ifdef AD_MUTANT_ALIGNMENT
 subq $16,%rsp
#else
 subq $24,%rsp
#endif
 movq $7,0(%rsp)
 movq $8,8(%rsp)
 movq $1,%rdi
 movq $2,%rsi
 movq $3,%rdx
 movq $4,%rcx
 movq $5,%r8
 movq $6,%r9
 movq %rsp,%rax
 andq $15,%rax
 movq %rax,ad_before8(%rip)
 call ad_record8
#ifdef AD_MUTANT_ALIGNMENT
 addq $16,%rsp
#else
 addq $24,%rsp
#endif
 ret
.size ad_call8,.-ad_call8
.type ad_record8,@function
ad_record8:
 movq %rsp,%rax
 andq $15,%rax
 movq %rax,ad_entry8(%rip)
 movq 8(%rsp),%rax
 movq %rax,ad_seventh(%rip)
 movq 16(%rsp),%rax
 movq %rax,ad_eighth(%rip)
 jmp mi_mix
.size ad_record8,.-ad_record8
.globl ad_cfi
.type ad_cfi,@function
ad_cfi:
 .cfi_startproc
 pushq %rbx
 .cfi_def_cfa_offset 16
 .cfi_offset %rbx,-16
 subq $32,%rsp
#ifdef AD_BAD_CFI
 .cfi_def_cfa_offset 40
#else
 .cfi_def_cfa_offset 48
#endif
 testl %edi,%edi
 jz .Learly
 call ad_trace_leaf
 .cfi_remember_state
 addq $32,%rsp
 .cfi_def_cfa_offset 16
 popq %rbx
 .cfi_def_cfa_offset 8
 .cfi_restore %rbx
 ret
.Learly:
 .cfi_restore_state
 addq $32,%rsp
 .cfi_def_cfa_offset 16
 popq %rbx
 .cfi_def_cfa_offset 8
 .cfi_restore %rbx
 ret
 .cfi_endproc
.size ad_cfi,.-ad_cfi
.section .note.GNU-stack,"",@progbits
