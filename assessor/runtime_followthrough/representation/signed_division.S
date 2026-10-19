.syntax unified
.thumb
.text
.global __aeabi_ldivmod
.type __aeabi_ldivmod,%function
.thumb_func
__aeabi_ldivmod:
 push {r4,lr}
 sub sp,sp,#32
 stmia sp,{r0-r3}
 mov r0,sp
 add r1,sp,#16
 bl rv_signed_words
 ldrd r0,r1,[sp,#16]
 ldrd r2,r3,[sp,#24]
 add sp,sp,#32
 pop {r4,pc}
.size __aeabi_ldivmod,.-__aeabi_ldivmod
.section .note.GNU-stack,"",%progbits
