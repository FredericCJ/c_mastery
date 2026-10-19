.syntax unified
.thumb
.text
.global fb_branch
.type fb_branch,%function
.thumb_func
fb_branch:
 ldr r3,=0xE000ED08
 str r0,[r3]
 movs r3,#0
 msr control,r3
 dsb sy
 isb sy
 msr msp,r1
 bx r2
.size fb_branch,.-fb_branch
.section .note.GNU-stack,"",%progbits
