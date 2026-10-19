# T53.03 assessor analysis

The allocation model reserves whole half-open objects, checks integer end and
alignment arithmetic before publication, and treats a duplicate owner-kind as
invalid even if its ranges differ. A nominal per-CPU index is not proof of
distinct backing storage. The actual target logs and independently compares
every stack/table extent. CPU0 explicitly changes to its permanent stack too;
merely allocating a CPU0 idle stack while remaining on the boot stack would not
demonstrate the stated ownership contract.

The cooperative switch saves the SysV callee-save set. The interrupt path must
save all general registers that arbitrary interrupted code can have live.
Its vector/error pair normalizes hardware frames, then C sees exact saved RIP,
old RSP and stack ownership. IST1→IST2→IST3 is a declared finite nesting chain;
each unconditional IST load selects a different top. Reentering a vector using
the same unchanged top is unsafe. The two known faults recover only after exact
site/vector/error checks. These checks do not make arbitrary corrupt state safe.
Held fault progress is observed from peers using acquire/release counters,
separately from completion and from sampled canaries.

The SIMD variant saves512 bytes with FXSAVE64 and restores them on interrupt
return and cooperative context transfer. The C kernel remains general-register
only. Hardware feature support, OS-enable state and save-area ownership are
distinct. The deliberately corrupted XMM0/XMM15/MXCSR is repaired by the correct
interrupt restore; omitting that restore falsifies the preservation property.
This profile deliberately excludes AVX/YMM upper halves and all other extended
state. An asynchronous scheduler must own every live state at each interruption;
a cooperative ABI caller-save argument cannot justify dropping interrupted state.

Compiler stack-usage records cover each translated frame, not a whole nested
execution. For this fixed task tree there are five live `sp_nested` frames plus
the task caller and assembly probe/yield/switch frames. The interrupt, #UD and
#GP paths have separate stacks, each with40 hardware bytes,16 vector/error bytes,
120 saved GPR bytes, alignment slack, and optional512-byte extended-state area,
plus the actual C handler/callee frames. Record inlining and changed optimization.
Do not add the three separate IST budgets as if they occupied one stack, nor
infer a general bound from an intact canary. Changed-ABI probe depth is fixed;
its Arm compile does not execute an Arm interrupt or validate a board.

Cost evidence compares the exact same atomic operation count and final values.
Padding is about cache coherence traffic, not mutual exclusion. A negative or
noisy timing difference does not falsify correctness and cannot be hidden by
choosing the fastest sample. Physical core topology, scheduling interference,
frequency and cache geometry qualify interpretation. Host dependency use is
permitted only for this separate measurement driver.

Keep the original criterion text, baseline bindings and historical receipts.
The successor supplies author implementation/experiments, not a learner attempt
or external certification. Use the public objective-specific rubric and require
the separately delayed variants under changed assumptions.
