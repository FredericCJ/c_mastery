/* Unsolved adapter: replace the rejection with the documented stack/C-call bridge. */
.text
.global _start
.type _start,@function
_start:
    mov $93,%edi
    mov $60,%eax
    syscall
    ud2
.size _start,.-_start
.section .note.GNU-stack,"",@progbits
