/* Capture the actual function-entry stack before a C prologue can hide it. */
.section .text,"ax"
.code64
.global kernel_main,kf_entry_rsp
kernel_main:
 mov %rsp,kf_entry_rsp(%rip)
 jmp kf_boot_check
.section .bss
.align 8
kf_entry_rsp: .skip 8
.section .note.GNU-stack,"",@progbits
