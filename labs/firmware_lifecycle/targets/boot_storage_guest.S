.syntax unified
.thumb
.section .vectors,"a",%progbits
.global vectors
vectors:
 .word _stack_top
 .word Reset
 .word Fault,Fault,Fault,eg_busfault
 .rept 10
 .word Fault
 .endr
.section .text.start,"ax",%progbits
.global Reset
.thumb_func
Reset:
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
4: bl fb_guest_main
 b Fault
.thumb_func
Fault:
 movs r0,#4
 ldr r1,=fault_message
 bkpt 0xab
 movs r0,#0x20
 ldr r1,=failed_exit
 bkpt 0xab
 b .
.align 2
fault_message:.asciz "FB_GUEST_FAULT\n"
.align 2
failed_exit:.word 0x20026,1
.section .candidate,"ax",%progbits
.balign 512
.global app_start,app_end
app_start:
 .word _app_stack
 .word AppReset
 .rept 14
 .word AppFault
 .endr
 .space 448
.thumb_func
AppReset:
 ldr r0,=_app_data
 ldr r1,=0x12345678
 str r1,[r0]
 ldr r0,=_app_bss
 movs r1,#0
 str r1,[r0]
 ldr r0,=0xE000ED08
 ldr r0,[r0]
 ldr r1,=app_start
 cmp r0,r1
 bne AppFail
 mrs r0,control
 cmp r0,#0
 bne AppFail
 mrs r0,primask
 cmp r0,#1
 bne AppFail
 mrs r0,msp
 ldr r1,=_app_stack
 cmp r0,r1
 bne AppFail
 ldr r0,=0xE000ED94
 ldr r0,[r0]
 cmp r0,#5
 bne AppFail
#if FB_FAULT_VARIANT == 1
 ldr r0,=_app_data
 ldr r1,=0x47702000
 str r1,[r0]
 adds r0,r0,#1
 bx r0
#elif FB_FAULT_VARIANT == 2
 ldr r0,=app_start
 movs r1,#0
 str r1,[r0]
#endif
 movs r0,#4
 adr r1,app_message
 bkpt 0xab
 movs r0,#0x20
 adr r1,app_exit
 bkpt 0xab
 b .
.thumb_func
AppFail:
 movs r0,#4
 adr r1,app_fail
 bkpt 0xab
 movs r0,#0x20
 adr r1,app_bad_exit
 bkpt 0xab
 b .
.thumb_func
AppFault:
 movs r0,#4
 adr r1,app_fault
 bkpt 0xab
 movs r0,#0x20
#if FB_FAULT_VARIANT == 1 || FB_FAULT_VARIANT == 2
 adr r1,app_exit
#else
 adr r1,app_bad_exit
#endif
 bkpt 0xab
 b .
.balign 4
app_message:.asciz "FB_GUEST_HANDOFF_PASS:VTOR,MSP,CONTROL,PRIMASK,MPU\n"
app_fail:.asciz "FB_GUEST_HANDOFF_FAIL\n"
app_fault:.asciz "FB_GUEST_MPU_FAULT:original invalid access contained\n"
.balign 4
app_exit:.word 0x20026,0
app_bad_exit:.word 0x20026,1
.ltorg
.balign 4
app_end:
.section .note.GNU-stack,"",%progbits
