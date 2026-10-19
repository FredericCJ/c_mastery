.syntax unified
.thumb
.cpu cortex-m0
.text
.global ce_checked
.thumb_func
ce_checked:
push {r4-r7,lr}
sub sp,#20
str r2,[sp]
mov r3,r8
str r3,[sp,#4]
mov r3,r9
str r3,[sp,#8]
mov r3,r10
str r3,[sp,#12]
mov r3,r11
str r3,[sp,#16]
movs r4,#71
movs r5,#72
movs r6,#73
movs r7,#74
movs r3,#75
mov r8,r3
movs r3,#76
mov r9,r3
movs r3,#77
mov r10,r3
movs r3,#78
mov r11,r3
bl cm_ext
cmp r4,#71
bne .Lbad
cmp r5,#72
bne .Lbad
cmp r6,#73
bne .Lbad
cmp r7,#74
bne .Lbad
mov r3,r8
cmp r3,#75
bne .Lbad
mov r3,r9
cmp r3,#76
bne .Lbad
mov r3,r10
cmp r3,#77
bne .Lbad
mov r3,r11
cmp r3,#78
bne .Lbad
b .Ldone
.Lbad:
ldr r2,[sp]
movs r3,#1
str r3,[r2]
.Ldone:
ldr r3,[sp,#4]
mov r8,r3
ldr r3,[sp,#8]
mov r9,r3
ldr r3,[sp,#12]
mov r10,r3
ldr r3,[sp,#16]
mov r11,r3
add sp,#20
pop {r4-r7,pc}
