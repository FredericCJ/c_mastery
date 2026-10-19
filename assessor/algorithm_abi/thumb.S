.syntax unified
.cpu cortex-m4
.thumb
.text
.global ab_records_lower
.type ab_records_lower,%function
.thumb_func
ab_records_lower:
    push {r4-r6,lr}
    movs r3,#0
    mov r4,r1
.Lrecord_loop:
    cmp r3,r4
    bhs .Lrecord_done
    subs r5,r4,r3
    lsrs r5,r5,#1
    adds r5,r5,r3
    ldr r6,[r0,r5,lsl #3]
    cmp r6,r2
    blo .Lrecord_right
    mov r4,r5
    b .Lrecord_loop
.Lrecord_right:
    adds r3,r5,#1
    b .Lrecord_loop
.Lrecord_done:
    mov r0,r3
    pop {r4-r6,pc}
.size ab_records_lower,.-ab_records_lower

.global ab_callback_lower
.type ab_callback_lower,%function
.thumb_func
ab_callback_lower:
    push {r4-r11,lr}
    sub sp,sp,#12
    ldr r9,[sp,#48]
    mov r4,r0
    mov r5,r1
    movs r6,#0
    mov r7,r2
    mov r8,r3
.Lcallback_loop:
    cmp r6,r5
    bhs .Lcallback_done
    sub r10,r5,r6
    lsr r10,r10,#1
    add r10,r10,r6
    mov r0,r9
    mov r1,r4
    mov r2,r10
    blx r8
    cmp r0,r7
    blo .Lcallback_right
    mov r5,r10
    b .Lcallback_loop
.Lcallback_right:
    add r6,r10,#1
    b .Lcallback_loop
.Lcallback_done:
    mov r0,r6
#ifdef AB_MUTANT_CALLBACK
    mov r0,r12
#endif
    add sp,sp,#12
    pop {r4-r11,pc}
.size ab_callback_lower,.-ab_callback_lower

