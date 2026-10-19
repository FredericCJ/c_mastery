.syntax unified
.cpu cortex-m4
.thumb
.text
.global as_wide_abi_probe
.thumb_func
as_wide_abi_probe:
    ldr r12,=as_wide_insert
    b .Lprobe
.global as_wide_abi_insert_probe
.thumb_func
as_wide_abi_insert_probe:
    ldr r12,=algo_asm_insert
    b .Lprobe
.global as_wide_abi_sift_probe
.thumb_func
as_wide_abi_sift_probe:
    ldr r12,=algo_asm_sift
.Lprobe:
    push {r4-r11,lr}
    sub sp,sp,#20
    str r3,[sp]
    str r12,[sp,#4]
    mov r3,sp
    str r3,[sp,#8]
    movw r4,#0x1122
    movw r5,#0x2233
    movw r6,#0x3344
    movw r7,#0x4455
    movw r8,#0x5566
    movw r9,#0x6677
    movw r10,#0x7788
    movw r11,#0x8899
    bl .Lnested
    ldr r1,[sp]
    str r0,[r1]
    movs r0,#0
    ldr r1,[sp,#8]
    cmp sp,r1
    bne .Lrestore
    movw r1,#0x1122
    cmp r4,r1
    bne .Lrestore
    movw r1,#0x2233
    cmp r5,r1
    bne .Lrestore
    movw r1,#0x3344
    cmp r6,r1
    bne .Lrestore
    movw r1,#0x4455
    cmp r7,r1
    bne .Lrestore
    movw r1,#0x5566
    cmp r8,r1
    bne .Lrestore
    movw r1,#0x6677
    cmp r9,r1
    bne .Lrestore
    movw r1,#0x7788
    cmp r10,r1
    bne .Lrestore
    movw r1,#0x8899
    cmp r11,r1
    bne .Lrestore
    movs r0,#1
.Lrestore:
    add sp,sp,#20
    pop {r4-r11,pc}
.Lnested:
    push {r3,lr}
    mov r3,sp
    tst r3,#7
    bne .Lbad_alignment
    movw r3,#0xdead
    blx r12
    pop {r3,pc}
.Lbad_alignment:
    pop {r3,lr}
    movs r4,#0
    bx lr
