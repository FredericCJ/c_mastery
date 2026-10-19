.syntax unified
.cpu cortex-m0
.thumb
.text
.global ts_probe
.thumb_func
ts_probe:
  movs r0,#0
  bx lr
