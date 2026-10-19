.option norvc
.section .text.start
.globl _start
_start:
 la t0,__bss_start
 la t1,__bss_end
0: bgeu t0,t1,1f
 sb zero,0(t0)
 addi t0,t0,1
 j 0b
1:
 la sp,stack_top
 la t0,fail
 csrw mtvec,t0
 li t0,-1
 csrw pmpaddr0,t0
 li t0,15
 csrw pmpcfg0,t0
 li t0,0xb200
 csrw medeleg,t0
 la t0,trap
 csrw stvec,t0
 li t0,0x800
 csrw mstatus,t0
 la t0,supervisor
 csrw mepc,t0
 mret
supervisor:
 li s0,4656
 li s1,4657
 li s2,4658
 li s3,4659
 li s4,4660
 li s5,4661
 li s6,4662
 li s7,4663
 li s8,4664
 li s9,4665
 li s10,4666
 li s11,4667
 la a0,context
 call po_context_save
 bnez a0,restored
 li s0,0
 li s1,0
 li s2,0
 li s3,0
 li s4,0
 li s5,0
 li s6,0
 li s7,0
 li s8,0
 li s9,0
 li s10,0
 li s11,0
 la sp,alt_top
 la a0,context
 call po_context_restore
 j fail
restored:
 li t0,4656
 bne s0,t0,fail
 li t0,4657
 bne s1,t0,fail
 li t0,4658
 bne s2,t0,fail
 li t0,4659
 bne s3,t0,fail
 li t0,4660
 bne s4,t0,fail
 li t0,4661
 bne s5,t0,fail
 li t0,4662
 bne s6,t0,fail
 li t0,4663
 bne s7,t0,fail
 li t0,4664
 bne s8,t0,fail
 li t0,4665
 bne s9,t0,fail
 li t0,4666
 bne s10,t0,fail
 li t0,4667
 bne s11,t0,fail
 la t0,stack_top
 bne sp,t0,fail
 call rv_aggregate_experiment
 beqz a0,fail
 la t0,expected
 li t1,9
 sd t1,0(t0)
 la t0,expected_pc
 la t1,service_instruction
 sd t1,0(t0)
 csrsi sstatus,2
service_instruction:
 ecall
 csrr t0,sstatus
 andi t0,t0,2
 beqz t0,fail
 # Three-level Sv39 mappings. Identity code/stack and MMIO remain mapped.
 la t0,root_a
 li t1,0xcf
 sd t1,0(t0)
 li t1,0x200000cf
 sd t1,16(t0)
 la t1,middle_a
 srli t1,t1,2
 ori t1,t1,1
 sd t1,8(t0)
 la t0,root_b
 li t1,0xcf
 sd t1,0(t0)
 li t1,0x200000cf
 sd t1,16(t0)
 la t1,middle_b
 srli t1,t1,2
 ori t1,t1,1
 sd t1,8(t0)
 la t0,middle_a
 la t1,leaf_a
 srli t1,t1,2
 ori t1,t1,1
 sd t1,0(t0)
 la t0,middle_b
 la t1,leaf_b
 srli t1,t1,2
 ori t1,t1,1
 sd t1,0(t0)
 la t0,leaf_a
 la t1,page_a
 srli t1,t1,2
 ori t1,t1,0xc7
 sd t1,0(t0)
 la t0,leaf_b
 la t1,page_b
 srli t1,t1,2
 ori t1,t1,0x43
 sd t1,0(t0)
 la t0,page_a
 li t1,17
 sd t1,0(t0)
 la t0,page_b
 li t1,29
 sd t1,0(t0)
 la t0,root_a
 srli t0,t0,12
 li t1,0x8000000000000000
 or t0,t0,t1
 csrw satp,t0
 sfence.vma
 li t2,0x40000000
 ld t0,0(t2)
 li t1,17
 bne t0,t1,fail
 la t0,root_b
 srli t0,t0,12
 li t1,0x8000000000000000
 or t0,t0,t1
 csrw satp,t0
 sfence.vma
 ld t0,0(t2)
 li t1,29
 bne t0,t1,fail
 la t0,expected
 li t1,15
 sd t1,0(t0)
 la t0,expected_pc
 la t1,write_instruction
 sd t1,0(t0)
write_instruction:
 sd zero,0(t2)
 la t0,leaf_b
 sd zero,0(t0)
 sfence.vma
 la t0,expected
 li t1,13
 sd t1,0(t0)
 la t0,expected_pc
 la t1,read_instruction
 sd t1,0(t0)
read_instruction:
 ld t0,0(t2)
 la t0,trap_count
 ld t1,0(t0)
 li t0,3
 bne t0,t1,fail
 la a0,pass_msg
 li a1,0x5555
 j finish
.balign 4
trap:
 # Only these scratch registers are used by this synchronous teaching handler.
 addi sp,sp,-32
 sd t0,0(sp)
 sd t1,8(sp)
 sd t3,16(sp)
 csrr t0,sepc
 la t1,expected_pc
 ld t1,0(t1)
 bne t0,t1,fail
 csrr t0,scause
 la t1,expected
 ld t1,0(t1)
 bne t0,t1,fail
 li t1,9
 beq t0,t1,ecall_check
 csrr t0,stval
 li t1,0x40000000
 bne t0,t1,fail
 j trap_counting
ecall_check:
 csrr t0,sstatus
 andi t0,t0,0x122
 li t1,0x120
 bne t0,t1,fail
trap_counting:
 la t0,trap_count
 ld t1,0(t0)
 addi t1,t1,1
 sd t1,0(t0)
 csrr t0,sepc
 addi t0,t0,4
 csrw sepc,t0
 ld t0,0(sp)
 ld t1,8(sp)
 ld t3,16(sp)
 addi sp,sp,32
 sret
fail:
 la a0,fail_msg
 li a1,0x13333
finish:
 li t0,0x10000000
1: lbu t1,0(a0)
 beqz t1,2f
 sb t1,0(t0)
 addi a0,a0,1
 j 1b
2: li t0,0x100000
 sw a1,0(t0)
3: wfi
 j 3b
.section .rodata
pass_msg: .asciz "B06_RV_PASS context=12 trap=3 address_spaces=2\n"
fail_msg: .asciz "B06_RV_FAIL\n"
.section .bss
.balign 16
context: .space 112
expected: .space 8
expected_pc: .space 8
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
