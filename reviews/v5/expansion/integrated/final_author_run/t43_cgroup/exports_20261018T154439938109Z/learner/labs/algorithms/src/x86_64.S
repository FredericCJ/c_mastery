/* Learner kernels: implement invariant-preserving unsigned operations. */
.text
.globl algo_asm_lower
algo_asm_lower:
    xorl %eax,%eax
    ret
.globl algo_asm_insert
algo_asm_insert:
    ret
.globl algo_asm_sift
algo_asm_sift:
    ret
.section .note.GNU-stack,"",@progbits
