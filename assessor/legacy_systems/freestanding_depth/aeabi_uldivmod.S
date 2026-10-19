.syntax unified
.thumb
.text
.global __aeabi_uldivmod
.type __aeabi_uldivmod,%function
.thumb_func
__aeabi_uldivmod:
 push {r4,lr}
 sub sp,sp,#32
 stmia sp,{r0-r3}
 mov r0,sp
 add r1,sp,#16
 bl fr_div64_words
 ldrd r0,r1,[sp,#16]
 ldrd r2,r3,[sp,#24]
 add sp,sp,#32
 pop {r4,pc}
.size __aeabi_uldivmod,.-__aeabi_uldivmod
.global fr_test_divzero
.type fr_test_divzero,%function
.thumb_func
fr_test_divzero:
 push {r4,lr}
 movs r0,#7
 movs r1,#0
 movs r2,#0
 movs r3,#0
 bl __aeabi_uldivmod
 orrs r0,r0,r1
 orrs r0,r0,r3
 cmp r0,#0
 bne 1f
 cmp r2,#7
 bne 1f
 movs r0,#0
 pop {r4,pc}
1: movs r0,#1
 pop {r4,pc}
.size fr_test_divzero,.-fr_test_divzero
.section .note.GNU-stack,"",%progbits
