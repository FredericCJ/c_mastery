.syntax unified
.cpu cortex-m4
.thumb
.text
.global ab_thumb_register
.thumb_func
ab_thumb_register:
 movs r0,#0
 bx lr
