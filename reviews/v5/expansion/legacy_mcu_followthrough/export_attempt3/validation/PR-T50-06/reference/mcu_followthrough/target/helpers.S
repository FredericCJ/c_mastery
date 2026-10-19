.syntax unified
.thumb
.section .text.__aeabi_uldivmod,"ax",%progbits
.global __aeabi_uldivmod
.thumb_func
__aeabi_uldivmod:
 push {r4-r8,lr}
 mov r4,r0
 mov r5,r1
 mov r6,r2
 mov r7,r3
 bl mf_udiv64
 /* r = n - q*d (low64 product); unsigned quotient is already in r0:r1. */
 umull r2,r3,r0,r6
 mla r3,r0,r7,r3
 mla r3,r1,r6,r3
 subs r2,r4,r2
 sbc r3,r5,r3
 pop {r4-r8,pc}
.section .text.__aeabi_uidivmod,"ax",%progbits
.global __aeabi_uidivmod
.thumb_func
__aeabi_uidivmod:
 push {r4-r6,lr}
 mov r4,r0
 mov r5,r1
 bl mf_udiv32
 mul r1,r0,r5
 subs r1,r4,r1
 pop {r4-r6,pc}
.section .note.GNU-stack,"",%progbits
