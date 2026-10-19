#ifndef SS_BASE
#define SS_BASE 0x8000
#endif
/* Each macro instance is assembled for its own page. Changing SS_BASE rebuilds
 * all far targets, GDT pointers and mailbox references; no runtime byte patch. */
.macro SS_TRAMP number,page
.section .rodata.ss_ap,"a"
.code16
.global ss_ap_\number\()_start,ss_ap_\number\()_end
ss_ap_\number\()_start:
 cli
 cld
 xor %ax,%ax
 mov %ax,%ds
 mov %ax,%es
 mov %ax,%ss
 lgdt \page+(ss_gdtr_\number-ss_ap_\number\()_start)
 mov %cr0,%eax
 or $1,%eax
 mov %eax,%cr0
 ljmpl $24,$\page+(ss_pm_\number-ss_ap_\number\()_start)
.code32
ss_pm_\number:
 mov $16,%ax
 mov %ax,%ds
 mov %ax,%es
 mov %ax,%ss
 mov %cr4,%eax
 or $0x20,%eax
 mov %eax,%cr4
 mov \page+0xf00,%eax
 mov %eax,%cr3
 mov $0xc0000080,%ecx
 rdmsr
 or $0x900,%eax
 wrmsr
 mov %cr0,%eax
 or $0x80010000,%eax
 mov %eax,%cr0
 ljmpl $8,$\page+(ss_long_\number-ss_ap_\number\()_start)
.code64
ss_long_\number:
 mov \page+0xf08,%rsp
 mov \page+0xf18,%edi
 mov \page+0xf20,%esi
 mov \page+0xf28,%edx
 mov $\page,%ecx
 lea -8(%rsp),%r8
 mov \page+0xf10,%rax
 call *%rax
1: cli
 hlt
 jmp 1b
.align 8
ss_gdt_\number:
.quad 0,0x00af9a000000ffff,0x00cf92000000ffff,0x00cf9a000000ffff
ss_gdtr_\number:
.word 31
.long \page+(ss_gdt_\number-ss_ap_\number\()_start)
ss_ap_\number\()_end:
.endm
SS_TRAMP 0,SS_BASE
SS_TRAMP 1,SS_BASE+0x1000
SS_TRAMP 2,SS_BASE+0x2000
.text
.code64
.global ss_park
.type ss_park,@function
ss_park:
 /* Abandon bootstrap frame only after all mailbox fields were copied. No
    return or subsequent instruction reads that stack or low-memory page. */
 mov %rsi,%rsp
 xor %ebp,%ebp
 call ss_parked
2: cli
 hlt
 jmp 2b
.size ss_park,.-ss_park
.section .note.GNU-stack,"",@progbits
