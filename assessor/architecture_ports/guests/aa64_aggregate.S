.text
.global aa_aggregate_experiment
aa_aggregate_experiment:
 stp x29,x30,[sp,#-48]!
 add x8,sp,#16
 mov x0,#1
 mov x1,#2
 mov x2,#3
 mov x3,#4
 mov x4,#5
 mov x5,#6
 mov x6,#7
 mov x7,#8
 bl po_aggregate
 ldp x0,x1,[sp,#16]
 cmp x0,#36
 b.ne agg_fail
 cmp x1,#9
 b.ne agg_fail
 ldr x0,[sp,#32]
 cmp x0,#8
 b.ne agg_fail
 mov w0,#1
 b agg_done
agg_fail:
 mov w0,#0
agg_done:
 ldp x29,x30,[sp],#48
 ret
