/* NUCLEO-F439ZI board harness: vector table and reset entry.
 *
 * 16 system entries plus 91 external interrupts (STM32F42x/43x). Only
 * SysTick and EXTI9_5 (IRQ 23) have real handlers; every other exception is
 * recorded in the retained boot record by C code and reported at the next
 * boot. The reset entry copies .data from its flash load address, zeroes
 * .bss, points VTOR at this table and calls bf_main, which never returns.
 */
    .syntax unified
    .cpu cortex-m4
    .thumb

    .section .vectors,"a",%progbits
    .balign 512
    .global bf_vectors
bf_vectors:
    .word _estack
    .word bf_reset
    .word bf_unexpected_entry   /* 2 NMI */
    .word bf_fault_entry        /* 3 HardFault */
    .word bf_fault_entry        /* 4 MemManage */
    .word bf_fault_entry        /* 5 BusFault */
    .word bf_fault_entry        /* 6 UsageFault */
    .word 0, 0, 0, 0            /* 7-10 reserved */
    .word bf_unexpected_entry   /* 11 SVCall */
    .word bf_unexpected_entry   /* 12 DebugMonitor */
    .word 0                     /* 13 reserved */
    .word bf_unexpected_entry   /* 14 PendSV */
    .word bf_systick_handler    /* 15 SysTick */
    .rept 23                    /* IRQ 0-22 */
    .word bf_unexpected_entry
    .endr
    .word bf_exti9_5_handler    /* IRQ 23 EXTI9_5 */
    .rept 67                    /* IRQ 24-90 */
    .word bf_unexpected_entry
    .endr
    .size bf_vectors, . - bf_vectors

    .section .text.bf_reset,"ax",%progbits
    .global bf_reset
    .type bf_reset, %function
    .thumb_func
bf_reset:
    ldr r0, =_estack            /* also correct when entered by a debugger */
    mov sp, r0
    ldr r0, =_sidata
    ldr r1, =_sdata
    ldr r2, =_edata
1:  cmp r1, r2
    bhs 2f
    ldr r3, [r0], #4
    str r3, [r1], #4
    b 1b
2:  ldr r1, =_sbss
    ldr r2, =_ebss
    movs r3, #0
3:  cmp r1, r2
    bhs 4f
    str r3, [r1], #4
    b 3b
4:  ldr r0, =0xE000ED08         /* SCB_VTOR */
    ldr r1, =bf_vectors
    str r1, [r0]
    dsb
    isb
    bl bf_main
5:  b 5b
    .size bf_reset, . - bf_reset

    /* Pass the stacked exception frame (MSP or PSP per EXC_RETURN bit 2). */
    .section .text.bf_fault_entry,"ax",%progbits
    .global bf_fault_entry
    .type bf_fault_entry, %function
    .thumb_func
bf_fault_entry:
    tst lr, #4
    ite eq
    mrseq r0, msp
    mrsne r0, psp
    b bf_fault_record
    .size bf_fault_entry, . - bf_fault_entry

    .section .text.bf_unexpected_entry,"ax",%progbits
    .global bf_unexpected_entry
    .type bf_unexpected_entry, %function
    .thumb_func
bf_unexpected_entry:
    mrs r0, ipsr
    b bf_unexpected_record
    .size bf_unexpected_entry, . - bf_unexpected_entry

    .section .note.GNU-stack,"",%progbits
