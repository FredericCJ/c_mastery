.syntax unified
.cpu cortex-m4
.thumb
.text
.global ab_snippet
.thumb_func
ab_snippet:
 cmp r2,#5
1: bne.n 1b
 movs r0,#0xaa
