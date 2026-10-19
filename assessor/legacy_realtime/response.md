# Response-analysis technical argument

The inspected book recurrence lacks blocking and release jitter. The extended
exercise instead states an explicit busy-interval arrival bound, then derives
the monotone iteration from it. This is deliberately narrower than asserting a
general theorem for arbitrary task activation, carry-in or self-suspension.
Deadline rejection is a failed sufficient bound; only a matching actual trace
demonstrates a miss. A resource-limit return is unresolved. The zero-work/no-
blocking/no-overhead target completes without waiting for hypothetical higher
jobs, so it has an explicit zero result before recurrence iteration.

The reference divides a and b separately, sums integer quotients and accounts
for a carry in the remainder sum without constructing a+b. For example,
ceil((MAX+MAX)/MAX)=2 while a native64 numerator cannot represent2*MAX.
The checked restoring divider and shift/add product avoid a hidden64-bit
compiler division dependency on Cortex-M4. Generated code must still be audited:
source appearance does not establish actual helper closure.

Only a local result is mutated before a publishable status. INVALID/OVERFLOW
leave every output field unchanged, including after several valid iterations.
CONVERGED/EXCEEDED/LIMIT publish their full trace. Limits use actual computed
iterations; declaring the last capped estimate converged is the explicit
negative control. Input period/deadline validation applies to every supplied
task, while interference uses only indices preceding the target. The caller
owns the priority/model argument and must not mistake array order for a proved
optimal priority assignment.

The original case gives3→4→4. J=1 gives3→4→5→5; D=4 returns the first exceeded
estimate5. Per-job overhead target1/interferer1 changes base to4 and the bound
to8. Separate C1/T3 interrupt interference gives3→5→7→8→8 for the stated task
set. Charging that interrupt again in B or O would double count it. Conversely,
using only a per-job O to cover unbounded preemption overhead omits a proof
obligation. Keep actual physical measurements unavailable until observed.

The Python oracle uses arbitrary integers and direct ceiling arithmetic, so it
does not share the C restoring implementation. Its finite generated cases and
semantic mutants establish only the documented observations, not arbitrary
schedule completeness, standards conformance of every toolchain or mastery.
