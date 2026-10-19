.intel_syntax noprefix
.text
.globl ab_snippet
ab_snippet:
 .byte 0x48,0x3d
 .long -1
1: ja 1b
 cmp rax,0x12345678
 cmp al,0x7f
 cmp rax,-1
.section .note.GNU-stack,"",@progbits
