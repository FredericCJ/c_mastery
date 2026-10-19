.syntax unified
.thumb
.text
.global aa_isa_compare
.type aa_isa_compare,%function
.thumb_func
aa_isa_compare:
 cmp r0,r1
 mrs r2,APSR
 mov.w r3,#0
 bcs.n 1f
 orr r3,r3,#1
1: cmp r0,r1
 bls.n 2f
 orr r3,r3,#2
2: cmp r0,r1
 bge.n 3f
 orr r3,r3,#4
3: cmp r0,r1
 ble.n 4f
 orr r3,r3,#8
4: cmp r0,r1
 bne.n 5f
 orr r3,r3,#16
5: ubfx r0,r2,#29,#1
 orr r3,r3,r0,lsl #5
 ubfx r0,r2,#28,#1
 orr r3,r3,r0,lsl #6
 ubfx r0,r2,#31,#1
 orr r0,r3,r0,lsl #7
 bx lr
.size aa_isa_compare,.-aa_isa_compare
