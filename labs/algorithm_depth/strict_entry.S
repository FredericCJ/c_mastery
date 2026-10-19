.text
.globl _start
.type _start,@function
_start:
 andq $-16,%rsp
 call ad_image
 ud2
.section .note.GNU-stack,"",@progbits
