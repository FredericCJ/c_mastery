.section .rodata.ap,"a"
.global cmk2_ap_start,cmk2_ap_end
cmk2_ap_start:
.byte 0xf4
cmk2_ap_end:
.text
.global cmk2_switch
cmk2_switch:
 ret
.section .note.GNU-stack,"",@progbits
