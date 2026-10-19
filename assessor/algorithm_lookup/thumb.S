.syntax unified
.cpu cortex-m4
.thumb
.text
.global al_lookup_kernel
.type al_lookup_kernel,%function
.thumb_func
al_lookup_kernel:
    push {r4-r8,lr}
    mov r4,r0
    mov r5,r1
    ldr r6,[r2,#4]
    ldr r7,[r2]
    movs r8,#0
    movs r0,#0
    str r0,[r3,#4]
    mvns r0,r0
    str r0,[r3]
.Lvisit:
    add r0,r3,#8
    str r6,[r0,r8,lsl #2]
    adds r8,r8,#1
    str r8,[r3,#4]
    add r0,r6,r6,lsl #1
    add r0,r4,r0,lsl #2
    ldr r1,[r0,#8]
    cbz r1,.Ldone
#ifdef AL_MUTANT_TOMBSTONE
    cmp r1,#2
    beq .Ldone
#endif
    cmp r1,#1
    bne .Lnext
    ldr r0,[r0]
    cmp r0,r7
    beq .Lfound
.Lnext:
    cmp r8,r5
    bhs .Ldone
    adds r6,r6,#1
    cmp r6,r5
    bne .Lvisit
    movs r6,#0
    b .Lvisit
.Lfound:
    str r6,[r3]
.Ldone:
    pop {r4-r8,pc}
.size al_lookup_kernel,.-al_lookup_kernel
.section .note.GNU-stack,"",%progbits
