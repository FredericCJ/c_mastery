.text
.globl _start,sd_before_call,sd_probe
.type _start,@function
_start:
 xor %ebp,%ebp
 mov %rsp,sd_original_sp(%rip)
 mov %rsp,%rdi
#ifdef SD_BAD_SAVED_SP
 add $8,%rdi
#endif
 and $-16,%rsp
#ifdef SD_BAD_ALIGNMENT
 sub $8,%rsp
#endif
 mov %rsp,sd_call_sp(%rip)
sd_before_call:
 call sd_probe
 mov %eax,%edi
 mov $60,%eax
 syscall
 ud2
.size _start,.-_start
.type sd_probe,@function
sd_probe:
 mov %rsp,sd_helper_sp(%rip)
 jmp sd_entry
.size sd_probe,.-sd_probe
.section .note.GNU-stack,"",@progbits
