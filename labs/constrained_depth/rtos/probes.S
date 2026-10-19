.syntax unified
.thumb
.fpu fpv4-sp-d16
.text
.global cr_irq0
.thumb_func
cr_irq0:
 mov r0,lr
 mrs r1,msp
 mrs r2,psp
 b cr_irq0_body
.global cr_irq1
.thumb_func
cr_irq1:
 mov r0,lr
 mrs r1,msp
 mrs r2,psp
 b cr_irq1_body
.section .text.cr_fp_probe,"ax",%progbits
.global cr_fp_probe
.thumb_func
cr_fp_probe:
 push {r4,lr}
 vpush {s16-s31}
 mov r4,r0
 vmov s16,r0
 vmov s17,r0
 vmov s18,r0
 vmov s19,r0
 vmov s20,r0
 vmov s21,r0
 vmov s22,r0
 vmov s23,r0
 vmov s24,r0
 vmov s25,r0
 vmov s26,r0
 vmov s27,r0
 vmov s28,r0
 vmov s29,r0
 vmov s30,r0
 vmov s31,r0
 movs r0,#1
 bl vTaskDelay
 vmov r0,s16
 cmp r0,r4
 bne 1f
 vmov r0,s17
 cmp r0,r4
 bne 1f
 vmov r0,s18
 cmp r0,r4
 bne 1f
 vmov r0,s19
 cmp r0,r4
 bne 1f
 vmov r0,s20
 cmp r0,r4
 bne 1f
 vmov r0,s21
 cmp r0,r4
 bne 1f
 vmov r0,s22
 cmp r0,r4
 bne 1f
 vmov r0,s23
 cmp r0,r4
 bne 1f
 vmov r0,s24
 cmp r0,r4
 bne 1f
 vmov r0,s25
 cmp r0,r4
 bne 1f
 vmov r0,s26
 cmp r0,r4
 bne 1f
 vmov r0,s27
 cmp r0,r4
 bne 1f
 vmov r0,s28
 cmp r0,r4
 bne 1f
 vmov r0,s29
 cmp r0,r4
 bne 1f
 vmov r0,s30
 cmp r0,r4
 bne 1f
 vmov r0,s31
 cmp r0,r4
 bne 1f
 movs r0,#1
 b 2f
1:
 movs r0,#0
2:
 vpop {s16-s31}
 pop {r4,pc}
