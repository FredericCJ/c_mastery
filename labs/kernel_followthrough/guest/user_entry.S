.text
.code64
.global kf_skip_guard,kf_probe_stack
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
.section .note.GNU-stack,"",@progbits
