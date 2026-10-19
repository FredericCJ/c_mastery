.section .text.start
.globl _start
_start:
 ldr x0,=__bss_start
 ldr x1,=__bss_end
0: cmp x0,x1
 b.hs 1f
 strb wzr,[x0],#1
 b 0b
1:
 ldr x0,=stack_top
 mov sp,x0
 mrs x0,CurrentEL
 cmp x0,#4
 b.ne fail
 ldr x0,=vectors
 msr vbar_el1,x0
 isb
 mov x19,#4675
 mov x20,#4676
 mov x21,#4677
 mov x22,#4678
 mov x23,#4679
 mov x24,#4680
 mov x25,#4681
 mov x26,#4682
 mov x27,#4683
 mov x28,#4684
 mov x29,#4685
 ldr x0,=context
 bl po_context_save
 cbnz w0,restored
 mov x19,#0
 mov x20,#0
 mov x21,#0
 mov x22,#0
 mov x23,#0
 mov x24,#0
 mov x25,#0
 mov x26,#0
 mov x27,#0
 mov x28,#0
 mov x29,#0
 ldr x0,=alt_top
 mov sp,x0
 ldr x0,=context
 bl po_context_restore
 b fail
restored:
 mov x0,#4675
 cmp x19,x0
 b.ne fail
 mov x0,#4676
 cmp x20,x0
 b.ne fail
 mov x0,#4677
 cmp x21,x0
 b.ne fail
 mov x0,#4678
 cmp x22,x0
 b.ne fail
 mov x0,#4679
 cmp x23,x0
 b.ne fail
 mov x0,#4680
 cmp x24,x0
 b.ne fail
 mov x0,#4681
 cmp x25,x0
 b.ne fail
 mov x0,#4682
 cmp x26,x0
 b.ne fail
 mov x0,#4683
 cmp x27,x0
 b.ne fail
 mov x0,#4684
 cmp x28,x0
 b.ne fail
 mov x0,#4685
 cmp x29,x0
 b.ne fail
 mov x0,sp
 ldr x1,=stack_top
 cmp x0,x1
 b.ne fail
 bl aa_simd_experiment
 cbz w0,fail
 bl aa_aggregate_experiment
 cbz w0,fail
 bl aa_tasks_experiment
 cbz w0,fail
 ldr x0,=expected
 mov x1,#0x15
 str x1,[x0]
 ldr x0,=expected_pc
 ldr x1,=service_return
 str x1,[x0]
 svc #0
service_return:
 # 39-bit VA, 4KiB granule, normal WB and device-nGnRE MAIR entries.
 ldr x0,=root_a
 ldr x1,=0x405
 str x1,[x0]
 ldr x1,=0x40000701
 str x1,[x0,#8]
 ldr x1,=middle_a
 orr x1,x1,#3
 str x1,[x0,#16]
 ldr x0,=root_b
 ldr x1,=0x405
 str x1,[x0]
 ldr x1,=0x40000701
 str x1,[x0,#8]
 ldr x1,=middle_b
 orr x1,x1,#3
 str x1,[x0,#16]
 ldr x0,=middle_a
 ldr x1,=leaf_a
 orr x1,x1,#3
 str x1,[x0]
 ldr x0,=middle_b
 ldr x1,=leaf_b
 orr x1,x1,#3
 str x1,[x0]
 ldr x0,=leaf_a
 ldr x1,=page_a
 mov x2,#0x703
 orr x1,x1,x2
 str x1,[x0]
 ldr x0,=leaf_b
 ldr x1,=page_b
 mov x2,#0x783
 orr x1,x1,x2
 str x1,[x0]
 ldr x0,=page_a
 mov x1,#17
 str x1,[x0]
 ldr x0,=page_b
 mov x1,#29
 str x1,[x0]
 mov x0,#0x4ff
 msr mair_el1,x0
 // DDI0487H.a: TG1=10 (4KiB), T1SZ=25, EPD1=1; lower fields unchanged.
 // Disabled TTBR1 still gets defined granule/size encodings.
 ldr x0,=0x80993519
 msr tcr_el1,x0
 ldr x0,=root_a
 msr ttbr0_el1,x0
 dsb sy
 tlbi vmalle1
 dsb sy
 isb
 mrs x0,sctlr_el1
 orr x0,x0,#1
 msr sctlr_el1,x0
 isb
 mov x4,#0x80000000
 ldr x0,[x4]
 cmp x0,#17
 b.ne fail
 ldr x0,=root_b
 dsb ishst
 msr ttbr0_el1,x0
 isb
 tlbi vmalle1
 dsb ish
 isb
 ldr x0,[x4]
 cmp x0,#29
 b.ne fail
 ldr x0,=expected
 mov x1,#0x25
 str x1,[x0]
 ldr x0,=expected_pc
 ldr x1,=write_instruction
 str x1,[x0]
 ldr x0,=expected_iss
 mov x1,#0x4f
 str x1,[x0]
write_instruction:
 str xzr,[x4]
 ldr x0,=leaf_b
 str xzr,[x0]
 dsb ishst
 tlbi vmalle1
 dsb ish
 isb
 ldr x0,=expected_pc
 ldr x1,=read_instruction
 str x1,[x0]
 ldr x0,=expected_iss
 mov x1,#7
 str x1,[x0]
read_instruction:
 ldr x0,[x4]
 ldr x0,=trap_count
 ldr x1,[x0]
 cmp x1,#3
 b.ne fail
 ldr x1,=pass_msg
 mov x0,#4
 hlt #0xf000
 ldr x1,=exit_ok
 mov x0,#0x20
 hlt #0xf000
 b .
.balign 2048
vectors:
 b fail
 .balign 128
 b fail
 .balign 128
 b fail
 .balign 128
 b fail
 .balign 128
 b trap
 .balign 128
 b fail
 .balign 128
 b fail
 .balign 128
 b fail
 .balign 128
 b fail
 .balign 128
 b fail
 .balign 128
 b fail
 .balign 128
 b fail
 .balign 128
 b fail
 .balign 128
 b fail
 .balign 128
 b fail
 .balign 128
 b fail
 .balign 128
trap:
 stp x0,x1,[sp,#-32]!
 stp x2,x3,[sp,#16]
 mrs x0,elr_el1
 ldr x1,=expected_pc
 ldr x1,[x1]
 cmp x0,x1
 b.ne fail
 mrs x0,esr_el1
 lsr x2,x0,#26
 ldr x1,=expected
 ldr x1,[x1]
 cmp x2,x1
 b.ne fail
 cmp x2,#0x15
 b.eq trap_counting
 and x0,x0,#0x7f
 ldr x1,=expected_iss
 ldr x1,[x1]
 cmp x0,x1
 b.ne fail
 mrs x0,far_el1
 mov x1,#0x80000000
 cmp x0,x1
 b.ne fail
 mrs x0,elr_el1
 add x0,x0,#4
 msr elr_el1,x0
trap_counting:
 ldr x0,=trap_count
 ldr x1,[x0]
 add x1,x1,#1
 str x1,[x0]
 ldp x2,x3,[sp,#16]
 ldp x0,x1,[sp],#32
 eret
fail:
 ldr x1,=fail_msg
 mov x0,#4
 hlt #0xf000
 ldr x1,=exit_fail
 mov x0,#0x20
 hlt #0xf000
 b .
.section .rodata
pass_msg: .asciz "B06_AA_PASS context=11 trap=3 address_spaces=2\n"
fail_msg: .asciz "B06_AA_FAIL\n"
.balign 8
exit_ok: .quad 0x20026,0
exit_fail: .quad 0x20026,1
.section .bss
.balign 16
context: .space 112
expected: .space 8
expected_pc: .space 8
expected_iss: .space 8
trap_count: .space 8
.balign 4096
root_a: .space 4096
root_b: .space 4096
middle_a: .space 4096
middle_b: .space 4096
leaf_a: .space 4096
leaf_b: .space 4096
page_a: .space 4096
page_b: .space 4096
.balign 16
stack: .space 4096
stack_top:
alt: .space 4096
alt_top:
