.text
.code64
.global kf_register_loop
kf_register_loop:
 push %rbx
 push %rbp
 push %r12
 push %r13
 push %r14
 push %r15
 mov $17,%rax
 mov $34,%rbx
 mov $51,%rcx
 mov $68,%rdx
 mov $85,%rbp
 mov $102,%rsi
 mov $119,%rdi
 mov $136,%r8
 mov $153,%r9
 mov $170,%r10
 mov $187,%r11
 mov $204,%r12
 mov $221,%r13
 mov $238,%r14
 mov $255,%r15
1:
 incq kf_turns(%rip)
 decq kf_remaining(%rip)
 jnz 1b
 cmp $17,%rax
 jne bad1
 cmp $34,%rbx
 jne bad2
 cmp $51,%rcx
 jne bad3
 cmp $68,%rdx
 jne bad4
 cmp $85,%rbp
 jne bad5
 cmp $102,%rsi
 jne bad6
 cmp $119,%rdi
 jne bad7
 cmp $136,%r8
 jne bad8
 cmp $153,%r9
 jne bad9
 cmp $170,%r10
 jne bad10
 cmp $187,%r11
 jne bad11
 cmp $204,%r12
 jne bad12
 cmp $221,%r13
 jne bad13
 cmp $238,%r14
 jne bad14
 cmp $255,%r15
 jne bad15
 xor %eax,%eax
 jmp done
bad1:
 mov $1,%eax
 jmp done
bad2:
 mov $2,%eax
 jmp done
bad3:
 mov $3,%eax
 jmp done
bad4:
 mov $4,%eax
 jmp done
bad5:
 mov $5,%eax
 jmp done
bad6:
 mov $6,%eax
 jmp done
bad7:
 mov $7,%eax
 jmp done
bad8:
 mov $8,%eax
 jmp done
bad9:
 mov $9,%eax
 jmp done
bad10:
 mov $10,%eax
 jmp done
bad11:
 mov $11,%eax
 jmp done
bad12:
 mov $12,%eax
 jmp done
bad13:
 mov $13,%eax
 jmp done
bad14:
 mov $14,%eax
 jmp done
bad15:
 mov $15,%eax
 jmp done
done:
 pop %r15
 pop %r14
 pop %r13
 pop %r12
 pop %rbp
 pop %rbx
 ret
.section .note.GNU-stack,"",@progbits
