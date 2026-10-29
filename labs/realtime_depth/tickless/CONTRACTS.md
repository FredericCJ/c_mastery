# T52.08 modular time, phase and tickless reprogramming

## 1. Identity and families

This required continuation implements `AX-PR-T52-08-T52.08.a`, `.b`, `.c` and
their original `-DELAYED` variants. Families A/B/C/D/F/G/H/I/J cover temporal
provenance, arithmetic construction, wrap/equality, control diagnosis, exhaustive
finite comparisons, interrupted programming, resource costs, changed clock
assumptions and actual producer/target composition. Preserve historical metadata
and the original forward-distance9 fixture.

## 2. Objectives and inspected references

Use the exact `R-LTICK-*` records in
`research/v5/expansion/legacy_realtime_tickless/resources.json`. Arm DDI0403E.e
B3.3 defines counter/reload and pending behavior; PM0214r10§4.5 describes the
selected Cortex-M4 registers and stopped-clock limitation. PM0214physical248
contains inconsistent single-shot N/N-1 wording, recorded in source_notes.json.
Do not silently turn that sentence into a physical exact-delay claim.

The inspected FreeRTOS11.2.0 tickless function provides a comparison for sleep
abort checks, partial periods and accounting for a pending tick exactly once.
No FreeRTOS source implements this exercise. QEMU10.2.1 machine/timer sources
identify the AN386 CMSDK timer mapping and emulator count semantics; they do
not establish STM32 board wiring or oscillator behavior. The actual retained
CMX sleep/tick/cancel functions are explicit producer imports.

## 3. Prerequisites and learner artifacts

Bring `PR-T50-01`/`PR-T50-05` timer/task state, `PR-T51-01` serialization and
`PR-T52-07` bounded evidence ledgers. Reuse actual completed CMX sleep/deadline
operations. Bring `PR-T27-01` reset/vector and `PR-T29-01` interrupt-register
knowledge for the separate target adapter. Predict raw modular differences,
phase-sensitive elapsed times and the first lost wake before running code.

## 4. Environment and dependency profile

Use C17, caller-owned live disjoint objects, and externally serialized core
operations. `struct tq_timer` is noncopyable after initialization; only its own sleep
tasks may inhabit the embedded CMX kernel. It must not acquire external CMX
objects or receive direct caller mutations. Its internal task/control pointers
are used only according to the supplied producer contract. Integer arithmetic
is ISO C; exception handlers, memory-mapped registers, saved PRIMASK, compiler
assembly and QEMU timing are separately declared target mechanisms.

Host Python integers, stdio and sanitizers are verification tools. Strict M4
images link the explicit core, CMX, course byte helpers and public harness;
the actual timer image additionally links the explicit port implementation.
No libc/libgcc/libatomic or hidden divide helper is allowed. Audit generated
symbols, disassembly, stack files, map and size. The target is the owned QEMU
MPS2-AN386 image, not a flashed board or a deployed system.

## 5. Inputs, operations and concrete work

Implement unsolved `tickless.c` and, for the actual adapter, `port.c`.

`tq_relation` accepts widths8,16,32 and a separate infinite flag. Equal dates
are EQUAL, positive forward distance below half the modulus is FUTURE, exactly
half is AMBIGUOUS and the other half is PAST under the stated provenance window.
A raw pair cannot reveal that an allegedly past date actually lies more than
half a cycle in the future. Establish the interval bound independently.
Reject out-of-width values and invalid widths/flags without changing output.

`tq_ticks(ms,num,den)` computes the exact ceiling of ms×num/(1000×den), for
tick frequency num/den Hz with each positive parameter≤1,000,000. Use quotient/
remainder decomposition so a representable result is not rejected merely because
the raw product would overflow. `tq_not_before(now_us,ms,num,den)` checks
now_us+1000×ms, then converts that absolute microsecond instant by ceiling onto
the tick grid. Zero duration still rounds the absolute instant onto that grid;
it does not create a negative or infinite delay. Unrepresentable results and
invalid arguments preserve output. Arithmetic overflow may be reported before
an invalid rate when the absolute instant itself cannot be constructed.

`tq_init` creates four actual CMX task slots dedicated to sleeps. `tq_wait`
admits a relative delay shorter than2^31 or an infinite wait. Delay0 publishes
an immediate successful sleep event. A slot with an uncollected result cannot
be reused. `tq_advance` accepts forward observations separated by less than2^31,
calls actual CMX tick processing and caches each newly ready sleep's observation
time/result. `tq_cancel` retains CMX_CLOSED as the retained producer's cancellation
result. `tq_collect` publishes it once; no result means EMPTY and unchanged
outputs. Infinite waits do not collide with numerical deadline0 or UINT32_MAX.

`tq_plan(now)` advances time and returns the earliest finite deadline with a
unique nonwrapping64-bit token. Only one plan may be outstanding. Actual hardware
programming occurs between plan and commit. Queue insertion/removal may occur
in that interval under explicit serialization. `tq_commit(plan,after,pending_irq)`
checks the exact plan, advances from the post-program clock observation, then
recomputes the earliest deadline. A changed earliest event, pending interrupt or
ready result returns RETRY, consumes the plan and forbids sleep. The adapter must
disarm and retry or dispatch; it must never spin indefinitely inside a masked
reprogramming loop. Stale/altered plans are invalid. `tq_abort` always removes a
prepared/armed state. At exhausted plan or CMX wait counters, refuse new work;
cancellation, result collection, manual advancement and retirement remain legal.

`struct tq_resume` stores the last always-on counter reading, a fixed count-to-local-tick
ratio and a fractional remainder. `tq_resume` requires the local clock to have
stopped and no outstanding hardware plan. The independent32-bit counter must
advance by less than2^31 counts between observations; this is an environmental
premise, not something modular subtraction can detect after arbitrary wraps.
Convert elapsed counts, retain the fraction and advance CMX once. Repeated use
of the same counter sample contributes zero new time. A changed clock frequency
requires a new, explicitly synchronized conversion state; mutating the ratio
while retaining a remainder is forbidden.

The `struct tp_clock` adapter owns CMSDK timer0 and SysTick in the dedicated AN386 image.
Timer0 counts continuously while SysTick is deliberately stopped. Configure
aligned word registers, preserve/restore PRIMASK and serialize queue/register
changes. `tp_at` accepts a proven future/equal absolute date after reading the
actual counter; `tp_wait` derives a relative date from that observation. Queue
mutation invalidates hardware programming. `tp_arm` makes one bounded attempt:
plan, program SysTick, optionally delay while masked for the original test,
read time/pending state, then commit. RETRY disarms and clears only the observed
SysTick pending state after accounting for elapsed time. Long deadlines use
24-bit chunks and require reevaluation after an early interrupt. The exact
minimum interval and software costs limit observed latency; no exact physical
N-cycle promise is made from the register example.

`tp_interrupt` stops the one-shot, advances actual CMX time and leaves dispatch/
rearming to its caller. `tp_stop` samples the independent counter after stopping
SysTick; `tp_resume` accounts from that sample. This deliberately disabled-clock
experiment does not enter a physical low-power mode or prove that a board's
chosen auxiliary clock remains powered in that mode. Shutdown disables owned
devices and abandons any hardware plan. Other actors may not reconfigure them.

While stopped, reject queue insertion, manual interrupt advancement, arming
and a second stop under the saved interrupt mask, preserving logical time and
the auxiliary sample. Only resume accounts the intervening raw interval. A
shutdown is a distinct retired state: resume cannot restart it; initialize again.
The caller may inspect raw/projected time but may not directly advance the
owned core between stop and resume. Otherwise the same interval can be counted
once by ordinary advancement and again by resume. The actual target regression
attempts every forbidden call after the independent counter has advanced and
checks exact single accounting after resume.

## 6. Necessary mechanisms and counterexamples

At now4294967292, deadline5 is nine ticks forward. Equality is due. Distance2^31
has no unique order. Exhaust all65,536 eight-bit pairs, retaining those distinctions
and the valid-horizon premise. Treating every unsigned subtraction as an order
is a misconception even if ordinary wrap examples pass.

For5ms ticks and request11ms, nominal ceiling is3 ticks. From a call at4.999ms,
the third edge is15ms, only10.001ms later. The absolute not-before policy selects
20ms instead. At32768Hz the relative ceiling is361 ticks; at1024/3Hz it is4.
Show where rounding occurs and prove that intermediate overflow is avoided.

With equal deadlines5,5 and later9 near wrap, both equal sleeps must publish at
the same observation. An earlier insertion after programming requires a new
expiry. A timer becoming pending while programming is masked must be accounted
before sleep. Disabling SysTick while retaining a running auxiliary counter
must not freeze logical elapsed time or count the same interval twice.

## 7. Required reasoning and costs

State the independent half-range provenance and observation premises. Separate
due-time ordering, sampled dispatch time, quantization and interrupt latency.
The conversion denominator is at most10^12 and residual multiplication below
10^18; prove those bounds, checked quotient multiplication and final addition.
The restoring division remainder therefore needs no65th bit. Infinite state,
token exhaustion and collected-result ownership must remain explicit.

Explain the two-phase plan/commit protocol, why rechecking only a cached earliest
deadline loses an insertion, and why acknowledging pending state before accounting
time loses a wake. All loops are bounded except deliberate test spin intervals
bounded by an advancing owned counter. Core queue scans cover four slots;
conversion uses64 restoring steps plus bounded multiplication. Report stack/code
size and actual target instruction closure separately from physical time/energy.
The timer queue does not prove the entire scheduler idle: its caller must also
ensure that unrelated runnable work is absent before entering sleep.

## 8. Deliverables

Submit core and explicit port implementations, eight-bit comparison table,
full-width conversion/phase derivation, plan/commit interleavings, actual CMX
producer identity, retained-fraction resume histories and target register/IRQ
traces. Include generated-code/link/stack audits and diagnoses of all semantic
controls. Keep public core/port and imported producer stubs unsolved in the
curriculum distribution.

## 9. Verification and pass/fail

Run relation, conversion, wrap, reprogram, resume and guards. The independent
oracle checks71,216 rows:65,536 eight-bit pairs,1,200 conversion cases,640
reprogramming interleavings and3,840 resume observations. Actual C output must
match each expected row. Semantic controls admit the half-range boundary, omit
equality dispatch, round down, ignore phase, omit reprogramming and double-count
resume intervals. Separate core and actual CMX stubs must compile and reject.

The actual target harness records deadlines and sampled dispatch counts for
wrap/equal wakes, earlier insertion, a pending interrupt during masked programming,
and a stopped SysTick with continued CMSDK counting. Its lateness bounds are
declared emulator test bounds, not physical deadline guarantees. Two adapter
controls retain old programming or omit the post-program check; separate port,
core and CMX stubs must reject. Compile failures, generic faults, sanitizer
diagnostics and harness timeouts do not satisfy a negative test. Raw traces,
commands and hashes belong in the producer report, not invented prose.

## 10. Objective-specific rubric

For `.a`, level0 orders arbitrary dates; level1 handles one wrap; level2 explains
every relation/infinite state and admission bound; level3 derives the exhaustive
eight-bit changed-width result and rejects unbounded provenance.
For `.b`, level0 promises15ms from any phase; level1 returns3; level2 implements
checked absolute not-before conversion with extremal inputs; level3 transfers
the rational frequency and accounts for fractions and changed clock assumptions.
For `.c`, level0 programs once; level1 observes an ordinary timer; level2 provides
actual CMX and target reprogram/pending traces with semantic controls; level3
derives and implements stopped-clock resume without double counting, while
separating physical clock/power assumptions from emulator observations.

## 11. Graduated hints and private analysis

Hint1: an infinite flag and a valid tick value are different state. Hint2: split
the arithmetic before multiplication. Hint3: time can advance after a register
write while interrupts remain masked. Hint4: store which auxiliary-clock sample
has already been consumed. Separate reference implementations and analysis are
`assessor/legacy_realtime/tickless.c`, `tickless_port.c` and `tickless.md`.

## 12. Changed-condition assessment

Keep the original seven-day delay. `.a-DELAYED` reduces width to8 and explicitly
enumerates all comparisons. `.b-DELAYED` uses32768Hz and1024/3Hz with zero/maximal
duration, requiring a new arithmetic derivation. `.c-DELAYED` stops the local
clock and resumes from an independent counter with fractional and wrap samples,
then performs the dedicated emulator's real two-timer experiment. These are
implemented preparation opportunities, not independent learner attempts.

## 13. Containment

All programs, task identities, counters and faults are original teaching objects
inside local processes or owned QEMU images. Only the two emulator timer devices
and local image are configured. No host scheduling/security settings, external
targets, deployed equipment, hardware flashing or real data is involved. A lost
wake or timing miss is a synthetic property violation, not demonstrated code
execution or privilege escalation.

## 14. Evidence status and limitations

Pilot observations, full campaigns, independent review, learner attempts and
physical measurements remain distinct. Exact current checks are produced by
`tools/run_tickless_depth.py` and its evidence checker. No whole FreeRTOS/kernel
audit, physical oscillator calibration, board sleep-state behavior or absolute
mastery is established by finite tests. Preserve the documented ST wording
inconsistency and unavailable website access instead of claiming those were
resolved by reading different material.
