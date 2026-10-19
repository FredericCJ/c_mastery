.text
.code64
.global dispatch,kf_dispatch_rsp,kf_bad_stack,kf_skip_guard,kf_probe_stack
dispatch:
 mov %rsp,kf_dispatch_rsp(%rip)
 jmp kf_dispatch
kf_bad_stack:
 mov $0x100010000,%rsp
 ud2
1: hlt
 jmp 1b
/* Original target assembly demonstrations; no host address is dereferenced. */
kf_skip_guard:
 mov %rsp,%r11
 mov $0x401deff0,%rsp
 movb $24,(%rsp)
 mov %r11,%rsp
 ret
kf_probe_stack:
 mov %rsp,%r11
 mov $0x40200000,%r10
2:
 sub $4096,%r10
 movb $24,(%r10)
 cmp $0x401de000,%r10
 jne 2b
 mov %r11,%rsp
 ret
.bss
.align 8
kf_dispatch_rsp: .skip 8
.section .note.GNU-stack,"",@progbits
