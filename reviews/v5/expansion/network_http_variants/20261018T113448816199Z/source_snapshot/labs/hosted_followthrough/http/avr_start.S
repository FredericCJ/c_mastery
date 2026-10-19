.section .vectors,"ax",@progbits
.global _start
_start:
  cli
  clr r1
  ldi r16,hi8(0x08ff)
  out 0x3e,r16
  ldi r16,lo8(0x08ff)
  out 0x3d,r16
  jmp main
