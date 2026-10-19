.syntax unified
.thumb
.section .vectors,"a",%progbits
.word 0x20020000
.word EGReset
.word EGFatal
.word EGFatal
.word EGFatal
.word eg_busfault
.rept 10
.word EGFatal
.endr
.text
.global EGReset
.thumb_func
EGReset:
 ldr r0,=_data_load
 ldr r1,=_data_start
 ldr r2,=_data_end
1: cmp r1,r2
 bhs 2f
 ldr r3,[r0],#4
 str r3,[r1],#4
 b 1b
2: ldr r1,=_bss_start
 ldr r2,=_bss_end
 movs r3,#0
3: cmp r1,r2
 bhs 4f
 str r3,[r1],#4
 b 3b
4: bl eg_guest_main
 b EGFatal
.thumb_func
EGFatal:
 movs r0,#4
 adr r1,fail
 bkpt 0xab
 b .
.balign 4
fail:.asciz "EG_GUEST_UNEXPECTED_FATAL\n"
.section .note.GNU-stack,"",%progbits
