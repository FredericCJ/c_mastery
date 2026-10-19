.text
.global rv_aggregate_experiment
rv_aggregate_experiment:
 addi sp,sp,-48
 sd ra,40(sp)
 addi a0,sp,8
 li a1,1
 li a2,2
 li a3,3
 li a4,4
 li a5,5
 li a6,6
 li a7,7
 li t0,8
 sd t0,0(sp)
 call po_aggregate
 ld t0,8(sp)
 li t1,36
 bne t0,t1,agg_fail
 ld t0,16(sp)
 li t1,9
 bne t0,t1,agg_fail
 ld t0,24(sp)
 li t1,8
 bne t0,t1,agg_fail
 li a0,1
 j agg_done
agg_fail:
 li a0,0
agg_done:
 ld ra,40(sp)
 addi sp,sp,48
 ret
