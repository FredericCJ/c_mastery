.text
.global aa_tasks_experiment
aa_tasks_experiment:
 stp x29,x30,[sp,#-16]!
 ldr x0,=task_a_context
 ldr x1,=task_entry
 str x1,[x0,#88]
 ldr x1,=task_a_top
 str x1,[x0,#96]
 mov x1,#17
 str x1,[x0]
 str x0,[x0,#8]
 ldr x1,=task_a_count
 str x1,[x0,#16]
 ldr x1,=task_a_state
 str x1,[x0,#24]
 ldr x0,=task_b_context
 ldr x1,=task_entry
 str x1,[x0,#88]
 ldr x1,=task_b_top
 str x1,[x0,#96]
 mov x1,#29
 str x1,[x0]
 str x0,[x0,#8]
 ldr x1,=task_b_count
 str x1,[x0,#16]
 ldr x1,=task_b_state
 str x1,[x0,#24]
 ldr x0,=task_a_guard
 mov x1,#0x55aa
 stp x1,x1,[x0]
 ldr x0,=task_b_guard
 stp x1,x1,[x0]
 ldr x0,=task_cycles
 str xzr,[x0]
task_schedule:
 ldr x0,=task_cycles
 ldr x1,[x0]
 cmp x1,#6
 b.eq task_check
 and x1,x1,#1
 ldr x2,=task_a_context
 ldr x3,=task_b_context
 cmp x1,#0
 csel x2,x2,x3,eq
 ldr x0,=task_selected
 str x2,[x0]
 mov x0,x2
 bl task_admit
 cbz w0,task_fail
 ldr x0,=task_main_context
 bl po_context_save
 cbnz w0,task_returned
 ldr x0,=task_selected
 ldr x0,[x0]
 bl po_context_restore
 b task_fail
task_returned:
 ldr x0,=task_cycles
 ldr x1,[x0]
 add x1,x1,#1
 str x1,[x0]
 b task_schedule
task_entry:
 sub sp,sp,#16
 str x19,[sp]
 /* Exact remaining240 bytes admitted before touching SP;241 refused. */
 mov x0,#240
 cmp x0,#240
 b.hi task_fail
 sub sp,sp,x0
 add sp,sp,x0
 mov x0,#241
 cmp x0,#240
 b.ls task_fail
task_step:
 ldr x0,[sp]
 cmp x0,x19
 b.ne task_fail
 ldr x0,[x22]
 cmp x0,#1
 b.ne task_fail
 ldr x0,[x21]
 add x0,x0,#1
 str x0,[x21]
 cmp x0,#3
 b.eq task_finish
 str xzr,[x22]
 mov x0,x20
 bl po_context_save
 cbnz w0,task_step
 ldr x0,=task_main_context
 bl po_context_restore
 b task_fail
task_finish:
 mov x0,#2
 str x0,[x22]
 ldr x0,=task_main_context
 bl po_context_restore
 b task_fail
task_check:
 ldr x0,=task_a_context
 bl task_admit
 cbnz w0,task_fail
 ldr x0,=task_b_context
 bl task_admit
 cbnz w0,task_fail
 /* Guarded duplicate-resume attempt: dead state rejects before restoration. */
 ldr x0,=task_a_state
 ldr x0,[x0]
 cmp x0,#2
 b.ne task_fail
 ldr x0,=task_b_state
 ldr x0,[x0]
 cmp x0,#2
 b.ne task_fail
 ldr x0,=task_a_count
 ldr x0,[x0]
 cmp x0,#3
 b.ne task_fail
 ldr x0,=task_b_count
 ldr x0,[x0]
 cmp x0,#3
 b.ne task_fail
 ldr x0,=task_a_guard
 ldp x1,x2,[x0]
 mov x3,#0x55aa
 cmp x1,x3
 b.ne task_fail
 cmp x2,x3
 b.ne task_fail
 ldr x0,=task_b_guard
 ldp x1,x2,[x0]
 mov x3,#0x55aa
 cmp x1,x3
 b.ne task_fail
 cmp x2,x3
 b.ne task_fail
 mov w0,#1
 b task_done
task_fail:
 /* Failure inside a task must return through the saved scheduler SP. */
 ldr x0,=task_main_context
 ldr x1,[x0,#96]
 cbz x1,task_early_fail
 mov sp,x1
task_early_fail:
 mov w0,#0
task_done:
 ldp x29,x30,[sp],#16
 ret
task_admit:
 ldr x1,[x0,#24]
 ldr x2,[x1]
 cbnz x2,task_refuse
 mov x2,#1
 str x2,[x1]
 mov w0,#1
 ret
task_refuse:
 mov w0,#0
 ret
.bss
.balign 16
task_main_context: .space 112
task_a_context: .space 112
task_b_context: .space 112
task_selected: .space 8
task_cycles: .space 8
task_a_count: .space 8
task_b_count: .space 8
task_a_state: .space 8
task_b_state: .space 8
.balign 16
task_a_guard: .space 16
task_a_stack: .space 256
task_a_top:
task_b_guard: .space 16
task_b_stack: .space 256
task_b_top:
