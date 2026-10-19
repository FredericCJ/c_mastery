.text
.globl ab_snippet
ab_snippet:
 /* Force the original accumulator/imm32 spelling explicitly; mnemonic GAS
  * chooses the shorter imm8 form for -1. These are assembler directives. */
 .byte 0x48,0x3d
 .long -1
1: ja 1b
 cmpq $0x12345678,%rax
 cmpb $0x7f,%al
 cmpq $-1,%rax
.section .note.GNU-stack,"",@progbits
