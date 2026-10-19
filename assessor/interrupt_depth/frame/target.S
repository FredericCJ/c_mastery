.syntax unified
.thumb
.fpu fpv4-sp-d16
.section .vectors,"a",%progbits
.balign 1024
.word __stack_top,xf_reset
.rept 14
.word xf_fault
.endr
.word xf_irq_a,xf_irq_b,xf_irq_c
.rept 237
.word xf_fault
.endr
.section .text.xf_reset,"ax",%progbits
.global xf_reset
.type xf_reset,%function
.thumb_func
xf_reset:
 cpsid i
 ldr r0,=__data_load
 ldr r1,=__data_start
 ldr r2,=__data_end
1: cmp r1,r2
 bcs 2f
 ldr r3,[r0],#4
 str r3,[r1],#4
 b 1b
2: ldr r1,=__bss_start
 ldr r2,=__bss_end
 movs r3,#0
3: cmp r1,r2
 bcs 4f
 str r3,[r1],#4
 b 3b
4: bl xf_target_main
 b xf_fault
.size xf_reset,.-xf_reset
.macro IRQ name,number
.section .text.\name,"ax",%progbits
.global \name
.type \name,%function
.thumb_func
\name:
 tst lr,#4
 ite eq
 mrseq r0,msp
 mrsne r0,psp
 mov r1,lr
 movs r2,#\number
 mrs r3,msp
 push {r4,lr}
 bl xf_capture
 pop {r4,pc}
.size \name,.-\name
.endm
IRQ xf_irq_a,16
IRQ xf_irq_b,17
IRQ xf_irq_c,18
.section .text.xf_trigger,"ax",%progbits
.global xf_trigger
.type xf_trigger,%function
.thumb_func
xf_trigger:
 push {r4,r5,r6,lr}
 sub r0,r0,r2,lsl#2
 msr psp,r0
 movs r3,#2
 msr control,r3
 isb
 cbz r1,5f
 ldr r3,=0x3f800000
 vmov s0,r3
 ldr r3,=0x41700000
 vmov s15,r3
5: ldr r4,=0xe000e200
 movs r5,#1
 movs r0,#1
 movs r1,#2
 movs r2,#3
 movs r3,#4
 cpsie i
 str r5,[r4]
.global xf_interrupted_begin
xf_interrupted_begin:
 dsb
 isb
 nop
 nop
 nop
 nop
.global xf_interrupted_end
xf_interrupted_end:
 cpsid i
 movs r0,#0
 msr control,r0
 isb
 pop {r4,r5,r6,pc}
.size xf_trigger,.-xf_trigger
.section .note.GNU-stack,"",%progbits
