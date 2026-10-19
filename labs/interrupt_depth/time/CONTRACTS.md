# T29.09–.10: coherent samples, incomplete streams, uncertain time

This required successor serves PR-T29-05 and its six original AX criteria and six
unchanged delayed variants. C17 is the implementation baseline. It follows
T29.01–.08 (`Xf`, `Xi`, `Xd`) and checked integer arithmetic/T11 runtime closure.
The CPFClock/P4 and Dp/PR-T31-03 comparisons are **late-return integrations** after
those components exist; they do not create a prerequisite cycle. The ordinary
exercises first use the contracts below and supplied callback traces. Host Python
and pthreads verify target-independent reasoning; they are not target dependencies.

## Mechanism and progression

A timestamp contains at least three separate claims: its representation was read
coherently, it refers to a specified event, and its conversion to another timebase
has bounded uncertainty. None implies that every event was retained. A queue may
drop three records while every surviving timestamp is correct. An ISR timestamp
is later than the event that asserted its interrupt; a deferred-service timestamp
adds scheduling and backlog delay. Calling all three “event time” hides different
quantities.

For a **coupled monotonic hardware counter**, read high, low, high. Reject if the
high words differ; accept otherwise only under a proved bound that permits at most
one low-word carry during the read. With `b` low bits, a sufficient bound is
`advance <= 2^b-1`; a one-bit high word is then adequate to detect that carry.
The accepted low word belongs to the unchanged high word at the low read's
linearization point. A resetting counter, an unbounded delay, or independently
written software words violates this proof's premises. `volatile` does not repair
those violations or impose atomic multiword publication in ISO C.

`XkIo` reads are serialized, linearized callbacks. A caller supplies the counter's
widths and a bound on its advance while the independent guard clock advances by
at most `max_guard_span`. Derive this bound from documented clock ratios and worst
case execution/preemption assumptions. `xk_snapshot` cannot authenticate it. It
checks widths, the declared bound, monotonic guard observations and actual guard
span. It performs at most eight attempts, each with two guard reads, two high
reads and one low read. Outputs are unchanged on failure. A reduced-width
fixture deliberately exceeds the allowed bound and must be refused before I/O.

Software publication needs a different contract. After P4, inspect actual
`CPFClock`: one writer, atomic high/low/sequence fields, sequentially consistent
operations, odd sequence during update, even sequence on publication, no sequence
wrap before retirement. A bounded reader may return “busy” when interrupting an
odd writer; it must not spin waiting for the interrupted writer to resume. The
integration deliberately pauses the writer after its low-word store. Equal high
words alone would wrongly accept this state. Explain why merely making the high
word volatile would not establish the SC proof or remove a non-atomic data race.

`XkSequence` tracks event identifiers, separately from timestamp coherence.
An externally justified maximum forward distance below `2^31` disambiguates
32-bit residues within one epoch. Duplicates, stale/out-of-bound inputs, epoch
changes and total-missing overflow leave state unchanged. First observation
establishes the baseline; it cannot reveal earlier losses. A full unobserved
sequence wrap defeats this inference even if the residue looks small.

## Interval derivation and ownership

Calibration stores an anchor tick `a`, reference-time anchor interval `[L,U]`,
integer nominal quantum `q` nanoseconds/tick, fractional rate-error bound `p` ppm
(`0 <= p <= 1000000`), and nonzero epoch. For acquisition ticks `[b,e]`, define
`n0=(b-a)q`, `n1=(e-a)q`, and `D(n)=ceil(n*p/1000000)`.
The reference-time interval is `[L+n0-D(n0), U+n1+D(n1)]`.
This follows monotonic elapsed time and the stipulated symmetric frequency-error
bound; quantization/synchronization error must already be included in the anchor
and acquisition intervals. It is not a measurement of oscillator accuracy.

Compute `D` with quotient/remainder so `n*p` cannot overflow. Check every scaled
delta and endpoint addition before committing the result. For a sample interval
`[S0,S1]` with event-to-sample delay `[d0,d1]`, event time lies within
`[S0-d1,S1-d0]`; reject underflow, do not silently clamp it. Strict order A-before-B
requires `A.upper < B.lower`. Touching intervals still permit simultaneous events.
Overlapping intervals may be ordered by a separate causal argument, but these
numeric bounds alone do not establish it. Different epochs require an explicit
cross-epoch calibration; the API refuses direct ordering.

All structs contain values, no owning pointers; callers own output storage and
callback context throughout a call. Inputs must point to live properly aligned
objects. No allocation occurs. `XkSequence` needs external serialization if shared.
API calls do not mask interrupts, stop DMA, synchronize clocks or authenticate
reset-domain fences. `XkInterval.epoch` identifies the calibration domain, not an
unforgeable capability. Space is constant; snapshot time is bounded by the chosen
attempt count and callback costs. Calibration uses checked 64-bit division;
on the selected M4 compiler profiles that introduces original T11 helper calls.

## Six assessed tasks

**T29.09.a — identify the measured event (A,B,F,J).** Implement `clock.h` in
`time.c`. Produce a timeline with device event ID, ISR entry, acquisition begin,
low read, acquisition end, deferred service and terminal DMA outcome. Execute the
supplied counter fixture and real `Xi`/`Xd` adapters. Explain which event each
timestamp represents and how three dropped records are detected independently.
Return a machine-readable interval table, a sequence ledger and a reasoning
dossier. Later compare the actual Dp zero-drift model with your interval model
without changing `DP_MODEL` into physical provenance. Delayed transfer uses two
independent clocks: derive a calibration interval from sync observations, include
one-way-delay uncertainty, then propagate rate error between observations.

**T29.09.b — bounded snapshot construction and falsification (A,B,C,D,F,I).**
The supplied fixture reads high `7,8,8,8` and low `UINT32_MAX-5,4`; each pair of
high reads surrounds one low read. Use at most two attempts and give the accepted
value, acquisition bracket and exact number of reads. Diagnose an always-accept
version and a version with no advancement check. Demonstrate why the separate
software writer `7:FFFFFFFF -> 7:0 -> 8:0` is outside the hardware-counter proof.
After P4 compare the actual SC sequence reader, including interrupted odd writer
and retirement. Delayed transfer shrinks the counter to two low bits and one high
bit: inspect counter states `3 -> 5 -> 8`, explain the equal-high ABA, and state
which bound or synchronization protocol can restore the argument.

**T29.09.c — uncertainty, overflow and physical ordering (B,C,F,H,I).**
Use anchor tick100, reference interval `[1000,1010]ns`, quantum10ns, rate bound
10000ppm, sample tick105 and event delay `[2,9]ns`. Derive both intervals by hand;
check using an independent integer/set oracle. Include touching, disjoint,
overlapping and different-epoch intervals and all checked arithmetic boundaries.
Compare integer quantum20ns with the original calibration after changing a
prescaler; preserve uncertainty provenance. Explain why observed sample maxima
or finitely many successful conversions cannot prove a physical latency bound.

**T29.10.a — named controlled schedules (D,F,G,J).** Run delayed service with
capacity-three `Xi`, cancel-before-complete/complete-before-stop, stop-before-
complete, stale generation, and a reset which clears software but retains an
external device/supervisor. Use actual prior components; preserve `Xd`'s quiescence
and exactly-once terminal rules. Supply a partial-order diagram and state/output
trace for each schedule. Delayed transfer changes reset domains; identify which
epoch, pending-write and fence assumptions survive. A numeric fence is not proof
that a real bus-master has stopped.

**T29.10.b — reproducible capture and instrument perturbation (B,F,H,I).**
Complete the board protocol and capture template linked below, including an
external event marker, entry and deferred markers, separate event sequence IDs,
logic-analyzer sample uncertainty, clock-source/prescaler/board revision, raw
files and firmware identities. Implement acquisition callbacks only for a
qualified documented target. The supplied MPS2 adapter uses an actual 32-bit
CMSDK counter with constant high zero in a bounded no-wrap run; it does not
implement a 64-bit peripheral. Execute it now; physical capture remains a separate
evidence dimension. Delayed transfer moves the marker from ISR entry to deferred
work and requires a newly named quantity, bounds and explanation of backlog.

**T29.10.c — claim audit under changed conditions (A,D,F,I,J).** Submit an evidence
table separating ISO semantics, compiler object facts, emulator behavior and
board observations. Include the original-helper omission link rejection and
object/ELF imports. Use `claim_cases.json` premises to classify unsupported claims
and specify the next discriminating experiment. Delayed transfer changes board
revision and buffer placement; requalify clock/interrupt/route/cache/errata facts
before reusing a physical claim. A model result remains useful within its premises.

## Execution and assessment

Run `python labs/interrupt_depth/time/run.py --build build/my-time` for public
checks. The compiling learner scaffold returns `XK_UNIMPLEMENTED`, so rejection
is expected until implemented. `--source` selects a learner implementation
explicitly. `target_build.py` requires named learner-produced prerequisite sources,
including original T11 division/runtime sources; there is no library fallback.
Use `--cc arm-none-eabi-gcc` or `--cc clang`, `--opt 0`/`3`. The target linker has
no default libraries. Audit every object and final image; explain dead-section
discard and the difference between unreferenced imports and reachable closure.

Required artifacts: implementations, deterministic callback traces, checked math
derivation, source/ABI/runtime audit, named schedule traces, cost table, evidence
classification and board capture protocol. Run positive, compiled semantic
negative, boundary, oracle and actual component tests; record compiler/version,
flags, source hashes, commands, raw outputs and exit status. Host sanitizers are
bounded diagnostics, not proofs. An unread assertion or uncompiled mutant is not
a successful semantic test. Preserve intentionally unfinished learner files.

Rubric: 0 = missing/incorrect; 1 = examples pass but premises, ownership or failure
reasoning are absent; 2 = correct implementation, independent expected results,
negative controls and justified contracts; 3 = changed-condition derivation and
independent diagnosis exposing an invalid assumption without copying a solution.
Score each of the six criteria separately; at least2 on each is author-package
readiness, not a learner mastery claim. Delayed assessment uses the original
minimum-day requirement and withheld assessor variants after an actual learner
attempt. No source, physical, external-review or learner evidence is fabricated.

Hints: (1) write the observation instants before writing arithmetic; (2) separate
representation coherence from event completeness and timebase identity; (3) prove
the maximum number of carries, then search for equal-high counterexamples outside
that bound; (4) derive interval endpoints from extreme allowed states and check
every addition before mutating outputs. Full technical analysis and changed-
condition answers are in the assessor package, excluded from learner exports.

Sources: the exact inspected QEMU v10.2.1 CMSDK timer/MPS2 implementation and C11
N1570 atomic-ordering passages are registered in
`research/v5/expansion/legacy_advanced_time/resources.json`. N1570 is an older
draft, not the published C17 standard. No C23-only facility is needed; its presence
would not supply timer coherence, clock calibration or a board timing guarantee.
See `curriculum/guides/interrupt-time-board-protocol.md` and
`fixtures/embedded/interrupt_time_capture.json`. All executable work is original
local teaching code; no deployed hardware, real data or external target is used.
