.syntax unified
.thumb
.global dt_boot
.type dt_boot,%function
.thumb_func
dt_boot:
  cpsid i
  ldr r0,=dt_data_start
  ldr r1,=dt_bss_end
  ldr r2,=0xa5a5a5a5
1: cmp r0,r1
  bhs 2f
  str r2,[r0],#4
  b 1b
2: b dt_reset
.size dt_boot,.-dt_boot
