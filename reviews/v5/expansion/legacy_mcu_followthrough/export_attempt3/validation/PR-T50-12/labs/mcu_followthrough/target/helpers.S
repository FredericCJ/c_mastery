.syntax unified
.thumb
.section .text.__aeabi_uldivmod,"ax",%progbits
.global __aeabi_uldivmod,__aeabi_uidivmod
.thumb_func
__aeabi_uldivmod:
 b mf_trap
.thumb_func
__aeabi_uidivmod:
 b mf_trap
.section .note.GNU-stack,"",%progbits
