.syntax unified
.cpu cortex-m4
.thumb
.text
.global ab_poison
.type ab_poison,%function
.thumb_func
ab_poison:
    ldr r3,[r0]
    adds r3,r3,#1
    str r3,[r0]
    mov r3,sp
    tst r3,#7
    beq .Laligned
    ldr r3,[r0,#4]
    adds r3,r3,#1
    str r3,[r0,#4]
.Laligned:
    ldr r12,[r0,#8]
    ldr r0,[r1,r2,lsl #2]
    mov r1,r12
    mov r2,r12
    mov r3,r12
    bx lr
.size ab_poison,.-ab_poison
