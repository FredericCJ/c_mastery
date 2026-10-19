.syntax unified
.cpu cortex-m4
.thumb
.text
.macro count
    ldr r0,[r4,#464]
    ldr r1,[r4,#468]
    adds r0,r0,#1
    mov.w r2,#0
    adcs r1,r1,r2
    str r0,[r4,#464]
    str r1,[r4,#468]
.endm
.global ah_cd_sink
.type ah_cd_sink,%function
.thumb_func
ah_cd_sink:
    push {r4-r11,lr}
    sub sp,sp,#4
    mov r4,r0
    mov r5,r1
.Ltop:
    ldr r6,[r4,#448]
    add r7,r5,r5
    adds r7,r7,#1
    cmp r7,r6
    bhs .Ldone
    add r8,r7,#1
    cmp r8,r6
    bhs .Lchosen
    count
    add r9,r4,r7,lsl #4
    add r10,r4,r8,lsl #4
    ldr r0,[r10]
    ldr r1,[r9]
    cmp r0,r1
    beq .Lsibling_tie
    ldr r2,[r4,#452]
    cbz r2,.Lsibling_min
    cmp r0,r1
    bgt .Lright
    b .Lchosen
.Lsibling_min:
    cmp r0,r1
    blt .Lright
    b .Lchosen
.Lsibling_tie:
    ldr r0,[r10,#12]
    ldr r1,[r9,#12]
    cmp r0,r1
    bhs .Lchosen
.Lright:
    mov r7,r8
.Lchosen:
    count
    add r9,r4,r5,lsl #4
    add r10,r4,r7,lsl #4
    ldr r0,[r10]
    ldr r1,[r9]
    cmp r0,r1
    beq .Lparent_tie
    ldr r2,[r4,#452]
    cbz r2,.Lparent_min
    cmp r0,r1
    bgt .Lswap
    b .Ldone
.Lparent_min:
    cmp r0,r1
    blt .Lswap
    b .Ldone
.Lparent_tie:
    ldr r0,[r10,#12]
    ldr r1,[r9,#12]
    cmp r0,r1
    bhs .Ldone
.Lswap:
    ldm r9,{r0-r3}
    ldm r10,{r6,r8,r11,r12}
    stm r9,{r6,r8,r11,r12}
    stm r10,{r0-r3}
#ifndef AH_MUTANT_INDEX
    add r0,r4,#256
    str r5,[r0,r11,lsl #2]
    str r7,[r0,r2,lsl #2]
#endif
    mov r5,r7
    b .Ltop
.Ldone:
    add sp,sp,#4
    pop {r4-r11,pc}
.size ah_cd_sink,.-ah_cd_sink
