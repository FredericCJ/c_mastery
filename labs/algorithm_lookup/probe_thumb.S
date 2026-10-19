.syntax unified
.cpu cortex-m4
.thumb
.text
.global al_lookup_abi
.thumb_func
al_lookup_abi:
    push {r4-r11,lr}
    sub sp,sp,#4
    mov r12,sp
    str r12,[sp]
    movw r4,#0x1122
    movw r5,#0x2233
    movw r6,#0x3344
    movw r7,#0x4455
    movw r8,#0x5566
    movw r9,#0x6677
    movw r10,#0x7788
    movw r11,#0x8899
    bl al_lookup_kernel
    movs r0,#0
    ldr r1,[sp]
    cmp sp,r1
    bne .Lend
    movw r1,#0x1122
    cmp r4,r1
    bne .Lend
    movw r1,#0x2233
    cmp r5,r1
    bne .Lend
    movw r1,#0x3344
    cmp r6,r1
    bne .Lend
    movw r1,#0x4455
    cmp r7,r1
    bne .Lend
    movw r1,#0x5566
    cmp r8,r1
    bne .Lend
    movw r1,#0x6677
    cmp r9,r1
    bne .Lend
    movw r1,#0x7788
    cmp r10,r1
    bne .Lend
    movw r1,#0x8899
    cmp r11,r1
    bne .Lend
    movs r0,#1
.Lend:
    add sp,sp,#4
    pop {r4-r11,pc}
.section .note.GNU-stack,"",%progbits
