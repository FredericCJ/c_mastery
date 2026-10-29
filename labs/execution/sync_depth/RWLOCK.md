# Reader ownership, phase admission and explicit upgrade

This required successor expands the original `PR-T51-12` criteria and delayed
variants without changing its fixture, baseline identities or accepted evidence.
Implement the unsolved `rwlock.c` and `rwphase.c` interfaces. The implementation
baseline is C17, with explicit completed CMX imports and caller-owned storage.
Native pthreads, CPU affinity, clocks, sanitizers and Python interval checking
are host verification tools; they are not dependencies of the freestanding cores.

## Prerequisites and supplied work

Bring completed `PR-T51-01` wait/result handling, `PR-T51-03` mutex ownership and
`PR-T51-11` identity/quiescence artifacts with source hashes. Inspect the actual
CMX `cmx_rw_*`, `rw_admit`, writer abandonment and `cmx_latch_*` functions. The
writer-preference adapter reuses the actual CMX reader/writer mechanism. The
phase/upgrade adapter derives new admission state while using actual CMX latch
waits and terminal results; it is not a claim that CMX already supports those
policies. Use `rwlock.h`, `rwphase.h`, both test files, `rw_native_tests.c`,
`rw_m4.c`, `run_rwlock.py` and `verify_rw_intervals.py`. Completed assessor
implementations and technical analysis are separately packaged.

## Original policy, input and expected trace

Replay the retained fixture: A takes a read share; B requests write; C requests
read; A releases. The selected policy admits A then B while C waits. Only after
B releases may C read. Implicit read-to-write upgrade, recursive reads and
recursive writes are rejected. Track identities rather than merely a reader
count: a nonowner must not release another participant's share.

`struct cd_rw` holds an actual `cmx_rwlock`, sixteen operation slots, a registration
bitmap and a reader reservation limit in 1..16. `cdr_begin` returns immediate
OK or BLOCKED with a ticket. Collect even an immediate OK using `cdr_result`
before using and later unlocking the resource. A ticket binds control, task
generation and nonwrapping operation serial. A successful collection establishes
the held state; a failed collection returns the slot to idle. Duplicate/old
tickets are stale. Cancel of a still-pending operation uses actual CMX cancel;
an already selected grant wins and must be collected and released.

A read reservation lasts through pending, terminal and held states, ending at
failed-result collection or release. Reject admission beyond the configured
limit before mutating the CMX object or output ticket. This conservative bound
also limits a whole cohort that CMX may wake together. At limit2, A and B hold
reads; C receives FULL with no new ownership. It is a bounded reservation
policy, not a claim that a machine integer overflow must occur in the core.
Reject entry if the participant's ownership counter cannot represent another
hold. Serial exhaustion refuses new operations; existing ownership cleanup
remains legal.

Queue two writers behind A and a reader behind them. Timeout or cancel the
first writer: C must still wait while the second writer remains. Remove the
last writer: C may join A's read phase immediately. The gate is derived from
remaining writers, not a sticky pending flag. Actual selected CMX results must
be cached before a later task operation can overwrite them. Do not perform raw
CMX operations on an outstanding ticket's task behind the adapter.

## Phase admission variant

`struct cmx_cpf` implements a fixed sixteen-participant blocking queue over one actual
CMX latch per pending request. It owns reader bits, one writer identity,
per-request arrival order, an optional upgrader identity and the previous
nonempty phase. Every operation is externally serialized with CMX progress.
`cpf_begin/result/cancel/unlock` follow the same one-ticket/one-hold discipline.
Phase and admission counters cannot wrap. Each new phase consumes at least one
previously admitted request; derive `phases <= admissions`. Exhaustion refuses
new requests while already admitted operations can complete and release.

When a writer finishes and readers are queued, admit the entire queued reader
cohort before the next writer. Writers are ordered by arrival relative to other
writers. During a read phase, a newly arriving reader can join only while no
writer or upgrader is pending. When the read cohort drains, admit the earliest
writer. With no demand from the opposite class, no artificial idle time is
required; distinguish an empty opposite phase from two competing nonempty
phases. This blocking implementation applies the phase admission properties
from the inspected manuscript; it does not import that paper's spin algorithm,
remote-memory-reference costs or real-time blocking bounds.

The concrete six-participant trace is R0, queued W1, R2, W3, R4; after R0
releases, W1 runs, then the R2/R4 cohort, then W3, then R5 that arrived during
the cohort. The expected nonempty phase sequence is R-W-R-W-R. Explain the
admission cut that prevents R5 from extending the cohort across W3. A changed
writer-preferred policy can preserve exclusion yet fail this phase guarantee.
Finite traces are not proof of fairness under arbitrary scheduling. State
assumptions: each granted owner eventually executes and releases, each pending
participant eventually runs to collect, no invalid raw-core mutation, and finite
critical-section work. No numerical time bound follows without execution and
scheduler bounds.

## Single-upgrader protocol

`cpf_upgrade` requires a collected read hold. Exactly one upgrader token may
exist, and no earlier writer may be pending when it is granted. A second
upgrader or an upgrade behind an existing writer returns DEADLOCK without
releasing the caller's read share. Explain the cycle that would arise if a
reader retained its share behind a writer that cannot run until that share is
released. An upgrade is an explicit additional operation, not a write request
silently accepted by the ordinary reader API.

The upgrader retains its share while other readers drain and closes admission
to new readers. When it is the only reader, convert that share into exclusive
writer ownership under the same serialization boundary; the ownership count
remains one. Later writers cannot bypass it. Timeout or cancellation abandons
the upgrade token but retains the original read share. Collect the selected
error, then release that share normally. The gate reopens according to remaining
requests. A sole reader can upgrade immediately even at an already-expired
deadline under CMX's acquisition-before-blocking rule; a strict end-to-end
deadline would be a different contract. Downgrade is not provided.

## Lifetime, failure and cost obligations

Controls, CMX kernel/tasks and output storage must be live, correctly typed and
disjoint. A task registers once, remains live through its pending/result/held
state, and cannot exit or reuse identity before unregistering. Reject unrelated
WAITING task admission. Controls cannot be copied after initialization. Core
calls, including progress/result reads, require a saved interrupt-mask port or
one external native mutex; this is neither a lock-free nor an interrupt-safe
ISO C abstraction by itself. The target experiment below is sequential logical
task execution and does not supply an SMP or IRQ-exclusion port.

Close refuses while ownership remains. Successful close freezes admission;
collect any terminal results and release registrations before destroy. Handle
checks require valid owner storage. Reinitializing the same address requires
external quiescence of every old handle; a task generation alone does not
identify a new control lifetime. No owner-death repair or asynchronous
termination is implemented here. Preserve last valid state on rejected input,
except that progress may settle already selected terminal results.

Give pre/postconditions for every operation, an ownership/wait membership
invariant, writer FIFO and reader-cohort arguments, and cleanup accounting.
Individual scans are O(N), with fixed N=16. A whole cohort wake is O(N²):
the actual CMX reader admission repeatedly calls its linear `first` scan,
and the phase adapter signals up to N latches, each using that scan. Cancellation
also recomputes actual priority state; derive its bound from the imported code
instead of calling the whole operation constant or linear. Distinguish wrapper
cost from imported producer cost.
Storage is O(N), plus native trace storage used only for verification. Record
actual `sizeof`, alignment, generated code and linker imports before byte-cost
claims. No heap allocation is permitted in either core. Changing N, reader
limits, counter types or cancellation semantics invalidates relevant arguments.

## Starvation prefix and multicore investigation

The `starvation` case maintains one active reader while a second reader arrives
before the first releases. The correct writer gate stops the first new reader
and admits the waiting writer when the original cohort drains. The deliberately
reader-preferred control permits 64 reader handoffs with the writer still
pending, then fails the progress assertion. This is a finite starvation prefix.
An abstract unbounded-counter policy can extend the pattern indefinitely;
the actual bounded implementation eventually retires exhausted counters. Do not
infer literal infinite execution from a finite experiment or ignore retirement
when making a bounded wait claim.

The native driver runs three actual reader threads and one writer, pinned only
to two CPUs already allowed to the process. It records actual CPU observations.
Linux/WSL CPU numbers do not establish distinct physical cores or measured
hardware cache traffic. Two separate one-round cohort tests force three live
reader intervals. Then five repetitions per implementation compare the actual
CMX mutex, writer-preferred adapter and phase adapter under equal work: each
reader performs200 reads of64 ordinary words; the writer performs200 updates
of those64 words. Readers require a consistent all-equal snapshot. The payload
body runs outside the scheduler-state mutex, so reader overlap is real and
writer exclusion/publication must be justified.

The state mutex protects admission, result checking and interval recording.
Release after the payload body and subsequent acquisition establish the native
publication chain. Condition waits recheck predicates. The ordinary CMX mutex
is the baseline producer, not a synthetic delay loop. Its all-exclusive policy
differs from the read-sharing policies: equal safety, payload work and lifetime
obligations do not imply identical fairness or cost. Rotate benchmark order and
retain all raw samples; do not require a particular implementation to be faster.

`CLOCK_MONOTONIC_RAW` samples include dispatch, queueing, trace recording and
join completion from a common start gate; thread creation occurs before it.
Output is printed after measurement. Wait-time sums include port-mutex waiting
and logical admission. Each interval has an independent start/end event; the
Python set-based verifier checks12,008 intervals, no writer overlap, exact work,
complete ownership and cohort size. It does not reimplement admission policy.
Discuss modeled payload traffic versus compiler transformations, cache-line
sharing, coherence transfers, scheduler/VM noise and sanitizer overhead. Hardware
cache counters and physical worst-case timing are not collected by this driver.

## Execution and assessment

Run `run_rwlock.py` with explicit completed `--execution-source`,
`--execution-include`, `--rwlock-source`, `--rwphase-source`, and `--kind`.
Writer cases are `policy`, `cancel`, `bounds`, `starvation`; phase cases are
`phases`, `upgrade`, `cancel`, `boundaries`. Native mode needs no case argument.
The supplied scaffold must compile and fail a named semantic assertion until
implemented. Save native output and run `verify_rw_intervals.py <log>`.
Malformed intervals, missing work, wrong CPU observations and mismatched
ownership must fail the independent verifier.

Use strict GCC/Clang debug, optimized and ASan/UBSan builds; native work also
requires supported GCC/Clang TSan observations. Serial preflight rejects
reader-barging and wrong-phase controls before starting threads, avoiding an
intentional native data race. A crash, timeout, compiler failure, sanitizer
diagnostic or generic error is not an intended semantic rejection. Retain exact
mutant definitions and commands.

`rw_m4.c` runs the same eight logical cases in an owned QEMU Cortex-M4 image.
Use C17 freestanding soft-float, no default libraries, and explicitly declared
course memory helpers only. Inspect undefined symbols, disassembly and link
map; reject hidden atomic/division/library helpers rather than assuming source
code is dependency-free. Ordinary serialized core state requires no lock-free
atomic claim. This image establishes target ABI and sequential execution only,
not actual reader concurrency, IRQ exclusion, context switching or physical
board behavior. Native and target evidence are separate.

Deliver D1 state/ownership/fairness and publication proofs; D2 implementations
and explicit producer source identities; D3 raw traces, independent expected
results, interval audit, controls and resource/cost analysis; D4 the original
named delayed variants at least seven days after source-bound study. `.a`
assesses preference counterexamples and phase admission; `.b` assesses bounded
ownership, pending-writer removal and single-token upgrade/cancellation; `.c`
assesses recorded exclusion, starvation prefix, equal-work comparison and real
CPU observations. Level0 is missing/fabricated evidence or violated invariants;
level1 is nominal success; level2 requires the complete declared mechanism and
boundaries; level3 requires an unaided delayed changed-condition solution with
new arguments. Reference tests do not certify absolute mastery.

Hints: derive the queued-writer predicate from live requests; separate a held
share from its completed acquisition ticket; snapshot the reader cohort at a
phase boundary; then draw the upgrader-versus-earlier-writer cycle. Assessor
analysis is withheld. Security scope is only original local programs, synthetic
data, bounded workloads and disposable emulation. An exclusion failure does
not demonstrate arbitrary code execution. Current source-bound campaigns alone
establish actual execution status; independent human review, delayed learner
assessment and physical observations remain distinct.
