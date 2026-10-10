/* Learner kernels. Preserve the base scalar AAPCS32 contract. */
.syntax unified
.cpu cortex-m4
.thumb
.text
.global algo_asm_lower
.thumb_func
algo_asm_lower:
    movs r0,#0
    bx lr
.global algo_asm_insert
.thumb_func
algo_asm_insert:
    bx lr
.global algo_asm_sift
.thumb_func
algo_asm_sift:
    bx lr
