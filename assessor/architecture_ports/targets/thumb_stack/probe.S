.syntax unified
.cpu cortex-m0
.thumb
#ifndef TS_MUTANT
#define TS_MUTANT 0
#endif
.text
.global ts_probe
.type ts_probe,%function
.thumb_func
ts_probe:
  push {r4-r7,lr}
  sub sp,#4
  mov r4,sp
  cmp r0,#128
#if TS_MUTANT == 2
  bhs reject
#else
  bhi reject
#endif
  ldr r5,=test_top
  subs r6,r5,r0
#if TS_MUTANT == 1
  movs r7,#3
#else
  movs r7,#7
#endif
  bics r6,r7
  ldr r7,=test_low
  cmp r6,r7
  blo reject
  mov sp,r6
  bl ts_alignment
  cmp r0,#1
  bne wrong_alignment
  cmp r6,r5
  beq no_store
  movs r7,#0x5a
  str r7,[sp,#0]
no_store:
  str r6,[r1]
  mov sp,r4
  movs r0,#1
  b done
wrong_alignment:
  mov sp,r4
reject:
  movs r0,#0
done:
  add sp,#4
  pop {r4-r7,pc}
.size ts_probe,.-ts_probe
.global ts_alignment
.type ts_alignment,%function
.thumb_func
ts_alignment:
  mov r0,sp
  movs r2,#7
  tst r0,r2
  bne misaligned
  movs r0,#1
  bx lr
misaligned:
  movs r0,#0
  bx lr
.size ts_alignment,.-ts_alignment
