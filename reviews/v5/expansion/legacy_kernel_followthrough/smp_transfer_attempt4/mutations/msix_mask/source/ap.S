/* Copied to physical 0x8000. Mailbox 0x8f00 is populated before each SIPI;
 * AP startup is acknowledged before the BSP reuses it. */
.section .rodata.ap,"a"
.code16
.global cmk2_ap_start,cmk2_ap_end
cmk2_ap_start:
 cli
 cld
 xor %ax,%ax
 mov %ax,%ds
 mov %ax,%es
 mov %ax,%ss
 lgdt 0x8000+(ap_gdtr-cmk2_ap_start)
 mov %cr0,%eax
 or $1,%eax
 mov %eax,%cr0
 ljmpl $24,$0x8000+(ap_pm-cmk2_ap_start)
.code32
ap_pm:
 mov $16,%ax
 mov %ax,%ds
 mov %ax,%es
 mov %ax,%ss
 mov 0x8f00,%eax
 mov %eax,%cr3
 mov %cr4,%eax
 or $0x20,%eax
 mov %eax,%cr4
 mov $0xc0000080,%ecx
 rdmsr
 or $0x900,%eax
 wrmsr
 mov %cr0,%eax
 or $0x80010000,%eax
 mov %eax,%cr0
 ljmpl $8,$0x8000+(ap_long-cmk2_ap_start)
.code64
ap_long:
 mov 0x8f08,%rsp
 mov 0x8f18,%edi
 mov 0x8f10,%rax
 call *%rax
1: cli
 hlt
 jmp 1b
.align 8
ap_gdt:
.quad 0,0x00af9a000000ffff,0x00cf92000000ffff,0x00cf9a000000ffff
ap_gdtr:
.word 31
.long 0x8000+(ap_gdt-cmk2_ap_start)
cmk2_ap_end:
.text
.code64
.global cmk2_switch
cmk2_switch:
 push %rbp
 push %rbx
 push %r12
 push %r13
 push %r14
 push %r15
 mov %rsp,(%rdi)
 mov %rsi,%rsp
 pop %r15
 pop %r14
 pop %r13
 pop %r12
 pop %rbx
 pop %rbp
 ret
.macro IRQ name,number
.global \name
\name:
 pushq $\number
 jmp cmk2_irq_common
.endm
IRQ cmk2_timer,240
IRQ cmk2_ipi,241
IRQ cmk2_tlb,242
IRQ cmk2_spurious,255
IRQ cmk2_fault,254
cmk2_irq_common:
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
 mov 120(%rsp),%rdi
 mov %rsp,%r12
 and $-16,%rsp
 call cmk2_interrupt
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
 add $8,%rsp
 iretq
.section .note.GNU-stack,"",@progbits
