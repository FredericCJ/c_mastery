.syntax unified
.thumb
.text
.globl ce_checked
.thumb_func
ce_checked:
push {r4-r11,lr}
sub sp,#12
str r2,[sp]
movw r4,#4660
movw r5,#4661
movw r6,#4662
movw r7,#4663
movw r8,#4664
movw r9,#4665
movw r10,#4666
movw r11,#4667
bl cm_ext
movw r3,#4660
cmp r4,r3
bne .Lbadabi
movw r3,#4661
cmp r5,r3
bne .Lbadabi
movw r3,#4662
cmp r6,r3
bne .Lbadabi
movw r3,#4663
cmp r7,r3
bne .Lbadabi
movw r3,#4664
cmp r8,r3
bne .Lbadabi
movw r3,#4665
cmp r9,r3
bne .Lbadabi
movw r3,#4666
cmp r10,r3
bne .Lbadabi
movw r3,#4667
cmp r11,r3
bne .Lbadabi
b .Ldoneabi
.Lbadabi:
ldr r2,[sp]
movs r3,#1
str r3,[r2]
.Ldoneabi:
add sp,#12
pop {r4-r11,pc}
