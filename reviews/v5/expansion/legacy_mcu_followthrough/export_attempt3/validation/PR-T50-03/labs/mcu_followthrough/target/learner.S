.syntax unified
.thumb
.section .text.mf_direct_switch,"ax",%progbits
.global mf_direct_switch,mf_direct_probe,mf_direct_enter
.thumb_func
mf_direct_switch:
 /* TODO: implement the exact public call-boundary stack ledger. */
 b mf_trap
.thumb_func
mf_direct_probe:
 b mf_trap
.thumb_func
mf_direct_enter:
 b mf_trap
.section .note.GNU-stack,"",%progbits
