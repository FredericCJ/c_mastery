.syntax unified
.thumb
.section .vectors,"a",%progbits
.word 0x20010000
.word reset_handler
.rept 9
.word fault_handler
.endr
.word svc_handler
.rept 4
.word fault_handler
.endr
.text
.thumb_func
.global fault_handler
fault_handler:
 b .
