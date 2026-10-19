/* Additive successor of assessor/kernel/boot.S; only named boundary label
 * and stack alignment repair. Frozen provider remains unchanged. */
.section .multiboot,"a"
.align 8
header:
.long 0xe85250d6,0,24,(-(0xe85250d6+24)) & 0xffffffff
.short 0,0
.long 8
.section .text.boot,"ax"
.code32
.global _start
_start:
 cli
 cld
 mov %eax,boot_magic
 mov %ebx,boot_info
 mov $boot_stack_top,%esp
 /* Identity-map 0..4 GiB with 2 MiB supervisor pages, including PCI MMIO. */
 mov $boot_pdpt,%eax
 or $3,%eax
 mov %eax,boot_pml4
 xor %ecx,%ecx
1:
 mov %ecx,%eax
 shl $12,%eax
 add $boot_pd,%eax
 or $3,%eax
 mov %eax,boot_pdpt(,%ecx,8)
 inc %ecx
 cmp $4,%ecx
 jne 1b
 xor %ecx,%ecx
2:
 mov %ecx,%eax
 shl $21,%eax
 or $0x83,%eax
 mov %eax,boot_pd(,%ecx,8)
 inc %ecx
 cmp $2048,%ecx
 jne 2b
 mov %cr4,%eax
 or $0x20,%eax
 mov %eax,%cr4
 mov $boot_pml4,%eax
 mov %eax,%cr3
 mov $0xc0000080,%ecx
 rdmsr
 or $0x900,%eax
 wrmsr
 lgdt boot_gdtr
 mov %cr0,%eax
 or $0x80010000,%eax
 mov %eax,%cr0
.global kf_mode_boundary
kf_mode_boundary:
 jmp long_start
.code64
long_start:
 mov $16,%ax
 mov %ax,%ds
 mov %ax,%es
 mov %ax,%ss
 xor %eax,%eax
 mov %ax,%fs
 mov %ax,%gs
 mov $boot_stack_top,%rsp
 mov boot_magic(%rip),%edi
 mov boot_info(%rip),%esi
 call kernel_main
3: cli
 hlt
 jmp 3b
.section .rodata
.align 8
boot_gdt:
.quad 0,0x00af9a000000ffff,0x00cf92000000ffff
boot_gdtr:
.word 23
.long boot_gdt
.section .bss
.align 4096
.global boot_pml4,boot_pdpt,boot_pd
boot_pml4: .skip 4096
boot_pdpt: .skip 4096
boot_pd: .skip 16384
.align 16
boot_magic: .skip 4
boot_info: .skip 4
.align 16 /* Saved arguments must not shift the C-call stack alignment. */
boot_stack: .skip 16384
boot_stack_top:
.section .note.GNU-stack,"",@progbits
