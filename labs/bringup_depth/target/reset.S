.syntax unified
.thumb
/* Learner supplies vector retention, table-driven copy/zero and entry policy. */
.section .text.reset,"ax",%progbits
.global bu_reset
.thumb_func
bu_reset:
 b bu_reset
.section .note.GNU-stack,"",%progbits
