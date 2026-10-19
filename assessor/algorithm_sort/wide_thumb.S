.syntax unified
.cpu cortex-m4
.thumb
.text
.global as_wide_insert
.type as_wide_insert,%function
.thumb_func
as_wide_insert:
 push {r4-r10,lr}
 movs r4,#1
.Lwide_outer:
 cmp r4,r1
 bhs .Lwide_done
 add r2,r0,r4,lsl #4
 ldr r6,[r2]
 ldr r7,[r2,#8]
#ifdef AS_MUTANT_WIDE
 movs r8,#0
#else
 ldr r8,[r2,#12]
#endif
 mov r5,r4
.Lwide_inner:
 cbz r5,.Lwide_place
 ldr r9,[r2,#-16]
 cmp r9,r6
 bls .Lwide_place
 ldr r10,[r2,#-8]
 ldr r3,[r2,#-4]
 str r9,[r2]
 str r10,[r2,#8]
 str r3,[r2,#12]
 sub r2,r2,#16
 subs r5,r5,#1
 b .Lwide_inner
.Lwide_place:
 str r6,[r2]
 str r7,[r2,#8]
 str r8,[r2,#12]
 adds r4,r4,#1
 b .Lwide_outer
.Lwide_done:
 pop {r4-r10,pc}
.size as_wide_insert,.-as_wide_insert
