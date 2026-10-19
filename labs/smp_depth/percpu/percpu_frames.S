/* Implement exact interrupt frames, controlled fault labels and context state. */
.text
.global sp_bsp_enter
sp_bsp_enter:
 jmp sp_bsp
.section .note.GNU-stack,"",@progbits
