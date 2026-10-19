.syntax unified
.thumb
.section .vectors,"a",%progbits
.global th_app_vectors
th_app_vectors:
.word _stack_top,th_app_reset
.rept 126
.word th_app_fault
.endr
.section .text.start,"ax",%progbits
.global th_app_reset
.thumb_func
th_app_reset:
 bl th_app_main
.thumb_func
th_app_fault: wfi
 b th_app_fault
.section .note.GNU-stack,"",%progbits
