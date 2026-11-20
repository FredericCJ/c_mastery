.file "entry.S"
.syntax unified
.cpu cortex-m4
.thumb
.section .vectors,"a",%progbits
.word _stack_top
.word reset
.text
.global reset
.thumb_func
reset:
    ldr r0, =_data_load
    ldr r1, =_data_start
    ldr r2, =_data_end
1:  cmp r1, r2
    bhs 2f
    ldr r3, [r0], #4
    str r3, [r1], #4
    b 1b
2:  ldr r1, =_bss_start
    ldr r2, =_bss_end
    movs r3, #0
3:  cmp r1, r2
    bhs 4f
    str r3, [r1], #4
    b 3b
4:  bl run_record
    b .
