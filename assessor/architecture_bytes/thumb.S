.syntax unified
.cpu cortex-m4
.thumb
.text
.global ab_thumb_register
.type ab_thumb_register,%function
.thumb_func
ab_thumb_register:
 movw r0,#0x7788
 movt r0,#0x5566
 movs r0,#0xaa
 bx lr
.size ab_thumb_register,.-ab_thumb_register
