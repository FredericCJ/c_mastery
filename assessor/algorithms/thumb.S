.syntax unified
.cpu cortex-m4
.thumb
.text
.global algo_asm_lower
.type algo_asm_lower,%function
.thumb_func
algo_asm_lower:
    push {r4-r6,lr}
    movs r3,#0
    mov r4,r1
.Llower_loop:
    cmp r3,r4
    bhs .Llower_done
    subs r5,r4,r3
    lsrs r5,r5,#1
    adds r5,r5,r3
    ldr r6,[r0,r5,lsl #2]
    cmp r6,r2
#ifdef ALGO_ASM_SIGNED
    blt .Llower_right
#else
    blo .Llower_right
#endif
    mov r4,r5
    b .Llower_loop
.Llower_right:
    adds r3,r5,#1
    b .Llower_loop
.Llower_done:
    mov r0,r3
    pop {r4-r6,lr}
#ifdef ALGO_ASM_ABI
    movs r4,#0
#endif
    bx lr
.size algo_asm_lower,.-algo_asm_lower
.global algo_asm_insert
.type algo_asm_insert,%function
.thumb_func
algo_asm_insert:
    push {r4-r8,lr}
    movs r4,#1
.Linsert_outer:
    cmp r4,r1
    bhs .Linsert_done
    add r2,r0,r4,lsl #3
    ldr r6,[r2]
    ldr r7,[r2,#4]
    mov r5,r4
.Linsert_inner:
    cbz r5,.Linsert_place
    sub r2,r2,#8
    ldr r8,[r2]
    cmp r8,r6
#ifdef ALGO_ASM_UNSTABLE
    blo .Linsert_found
#else
    bls .Linsert_found
#endif
    str r8,[r2,#8]
    ldr r3,[r2,#4]
    str r3,[r2,#12]
    subs r5,r5,#1
    b .Linsert_inner
.Linsert_found:
    adds r2,r2,#8
.Linsert_place:
    str r6,[r2]
    str r7,[r2,#4]
    adds r4,r4,#1
    b .Linsert_outer
.Linsert_done:
    pop {r4-r8,pc}
.size algo_asm_insert,.-algo_asm_insert
.global algo_asm_sift
.type algo_asm_sift,%function
.thumb_func
algo_asm_sift:
    push {r4-r8,lr}
    mov r4,r2
.Lsift_loop:
    lsls r5,r4,#1
    adds r5,r5,#1
    cmp r5,r1
    bhs .Lsift_done
    ldr r6,[r0,r5,lsl #2]
    adds r7,r5,#1
    cmp r7,r1
    bhs .Lsift_chosen
    ldr r8,[r0,r7,lsl #2]
    cmp r8,r6
    bls .Lsift_chosen
    mov r5,r7
    mov r6,r8
.Lsift_chosen:
    ldr r2,[r0,r4,lsl #2]
    cmp r2,r6
    bhs .Lsift_done
    str r6,[r0,r4,lsl #2]
    str r2,[r0,r5,lsl #2]
    mov r4,r5
    b .Lsift_loop
.Lsift_done:
    pop {r4-r8,pc}
.size algo_asm_sift,.-algo_asm_sift
