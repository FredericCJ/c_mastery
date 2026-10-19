.syntax unified
.thumb
.text
.global aw_lower
.type aw_lower,%function
.thumb_func
aw_lower:
 push {r4-r7,lr}
 movs r4,#0
 mov r5,r0
 mov r6,r3
.Law_loop:
 cmp r4,r1
 bhs .Law_done
 subs r3,r1,r4
 lsrs r3,r3,#1
 adds r3,r3,r4
 add r7,r5,r3,lsl #3
 ldr r0,[r7,#4]
 ldr r12,[r2,#4]
 cmp r0,r12
 beq .Law_id
 cmp r6,#0
 bne .Law_desc
 /* Direction test overwrote flags: reload the signed rank comparison. */
 cmp r0,r12
 blt .Law_less
 b .Law_high
.Law_desc:
 cmp r0,r12
 bgt .Law_less
 b .Law_high
.Law_id:
 ldr r0,[r7]
 ldr r12,[r2]
 cmp r0,r12
 blt .Law_less
.Law_high:
 mov r1,r3
 b .Law_loop
.Law_less:
 adds r4,r3,#1
 b .Law_loop
.Law_done:
 mov r0,r4
 pop {r4-r7,pc}
.size aw_lower,.-aw_lower
