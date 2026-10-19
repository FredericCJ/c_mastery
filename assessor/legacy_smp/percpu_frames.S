#ifndef SP_SIMD
#define SP_SIMD 0
#endif
#ifndef SP_GUEST_MUTANT
#define SP_GUEST_MUTANT 0
#endif
.text
.code64
.global sp_bsp_enter
sp_bsp_enter:
 mov %rdi,%rsp
 xor %ebp,%ebp
 call sp_bsp
1: cli
 hlt
 jmp 1b
.macro SP_ISR name,number,error
.global \name
\name:
.if !\error
 pushq $0
.endif
 pushq $\number
 jmp sp_irq_common
.endm
SP_ISR sp_probe_irq,244,0
SP_ISR sp_ud_irq,6,0
SP_ISR sp_gp_irq,13,1
sp_irq_common:
 cld
 push %rax
 push %rbx
 push %rcx
 push %rdx
 push %rbp
 push %rsi
 push %rdi
 push %r8
 push %r9
 push %r10
 push %r11
 push %r12
 push %r13
 push %r14
 push %r15
 mov %rsp,%r12
 mov %rsp,%rdi
 and $-16,%rsp
#if SP_SIMD
 sub $512,%rsp
 fxsave64 (%rsp)
 pxor %xmm0,%xmm0
 pxor %xmm15,%xmm15
 ldmxcsr sp_poison_mxcsr(%rip)
#endif
 call sp_interrupt
#if SP_SIMD
#if SP_GUEST_MUTANT != 2
 fxrstor64 (%rsp)
#endif
 add $512,%rsp
#endif
 mov %r12,%rsp
 pop %r15
 pop %r14
 pop %r13
 pop %r12
 pop %r11
 pop %r10
 pop %r9
 pop %r8
 pop %rdi
 pop %rsi
 pop %rbp
 pop %rdx
 pop %rcx
 pop %rbx
 pop %rax
 add $16,%rsp
 iretq
.global sp_raise_ud,sp_ud_site,sp_ud_return
sp_raise_ud:
sp_ud_site:
 ud2
sp_ud_return:
 ret
.global sp_raise_gp,sp_gp_site,sp_gp_return
sp_raise_gp:
 mov $0xdead,%eax
sp_gp_site:
 mov %ax,%ds
sp_gp_return:
 ret
.global sp_switch
sp_switch:
#if SP_SIMD
 fxsave64 (%rdx)
 fxrstor64 (%rcx)
#endif
 jmp cmk2_switch
.global sp_register_probe
sp_register_probe:
 push %rbp
 push %rbx
 push %r12
 push %r13
 push %r14
 push %r15
 sub $24,%rsp
 mov %rdi,%r12
 lea 1(%rdi),%r13
 lea 2(%rdi),%r14
 lea 3(%rdi),%r15
 mov %rdi,%rbx
 mov %rdi,%rax
 mov %rdi,%rcx
 mov %rdi,%rdx
 mov %rdi,%r8
 mov %rdi,%r9
 mov %rdi,%r10
 mov %rdi,%r11
#if SP_SIMD
 movq %r12,%xmm0
 movq %r13,%xmm15
 mov %r12d,%esi
 and $3,%esi
 shl $13,%esi
 or $0x1f80,%esi
 mov %esi,4(%rsp)
 ldmxcsr 4(%rsp)
#endif
 int $244
 cmp %r12,%rax
 jne sp_probe_bad
 cmp %r12,%rcx
 jne sp_probe_bad
 cmp %r12,%rdx
 jne sp_probe_bad
 cmp %r12,%r8
 jne sp_probe_bad
 cmp %r12,%r9
 jne sp_probe_bad
 cmp %r12,%r10
 jne sp_probe_bad
 cmp %r12,%r11
 jne sp_probe_bad
 call sp_yield
 cmp %r12,%rbx
 jne sp_probe_bad
 lea 1(%r12),%rax
 cmp %rax,%r13
 jne sp_probe_bad
 lea 2(%r12),%rax
 cmp %rax,%r14
 jne sp_probe_bad
 lea 3(%r12),%rax
 cmp %rax,%r15
 jne sp_probe_bad
#if SP_SIMD
 movq %xmm0,%rax
 cmp %r12,%rax
 jne sp_probe_bad
 movq %xmm15,%rax
 cmp %r13,%rax
 jne sp_probe_bad
 stmxcsr (%rsp)
 mov (%rsp),%eax
 cmp 4(%rsp),%eax
 jne sp_probe_bad
#endif
 xor %eax,%eax
 jmp sp_probe_return
sp_probe_bad:
 mov $1,%eax
sp_probe_return:
 add $24,%rsp
 pop %r15
 pop %r14
 pop %r13
 pop %r12
 pop %rbx
 pop %rbp
 ret
.section .rodata
.align 4
sp_poison_mxcsr:.long 0x5f80
.section .note.GNU-stack,"",@progbits
