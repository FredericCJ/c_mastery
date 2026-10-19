# T65.06 explicit scope and nested-frame analysis

J1 permitted comparison is a full controlling expression; initializer or summed
placement violates the inspected environmental limit even if accepted by a
compiler. J2's changed nonvolatile automatic local is indeterminate, not old or
new. J3 fixes that local-value issue only. J4 has a terminated destination;
caller-owned jmp_buf does not preserve its function. J5 violates same-thread
restoration. J6 exits the relevant variably modified scope. J7's caller context
is not a local automatic object of er_run, and its pointer parameter is unchanged.
J8 does not roll back resources or external counters. No undefined case is run.

The actual setjmp is in er_run, not ejd_run or either body. State s remains live
in ejd_run throughout both boundaries; its changed fields therefore are not the
problematic changed locals of er_run. Each invocation has its own frame. Inner
cleanup returns normally to its boundary, then outer continues while ID1 remains
owned. Raising from a cleanup or targeting an already returned inner invocation
would violate the contract; do not accept either as a shortcut.

Registration is an ownership transition. After take but before ow_defer, the
body must release directly before a jump; after registration the scope owns the
release responsibility. Before any acquisitions, inner installs its scope bridge
in the frame. The bridge runs once on normal or raised exit and pops the actual
OwScope; all entries are ALWAYS. Outer cleanup order5then1 is independent of
inner4then3then2. Cause conversion records both the inner code and stage; a flat
CONTRACT with erased stage is weaker evidence.

For fresh delayed assessment, make inner fail immediately after its third take,
then fail the outer continuation allocation. Require the learner to derive
inner THROWN/stage5, outer IO21, three persistent counter increments and cleanup
4,3,2,1. Allocation accounting and cleanup order must agree; no outer5 exists.
Then alter cleanup to be fallible: the present nonfailing release protocol no
longer suffices and a recovery owner/quiescence model is required. That broader
implementation is not silently claimed by this bounded task.

Original er_frame capacity exhaustion and every admitted error code are assessed
separately from allocation fault schedules. A finite symbolic lifetime oracle is
independent of saved-register implementation, not a proof of libc or arbitrary
nonlocal control. Author source/experimental evidence cannot establish delayed
unaided mastery or external-human review.
