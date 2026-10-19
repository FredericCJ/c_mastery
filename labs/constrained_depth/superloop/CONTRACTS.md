# Bounded superloop, owned continuations and release admission

## Identity and assessed objectives

This required successor covers all six `PR-T30-01` criteria, `T30.01.a/b/c`
and `T30.02.a/b/c`, with their original `-DELAYED` identifiers. It assesses
semantic derivation, construction, finite boundaries, diagnosis, falsification,
failure/recovery, cost and changed-condition transfer. Earlier hours and records
remain historical; this additional work has no study-time ceiling.

## Sources, prerequisites and dependency profile

Read the actual module, parent project and supplied fixture before these tasks.
The exact source register is
`research/v5/expansion/legacy_constrained_superloop/resources.json`. Its inspected
N1570 6.2.5 paragraph9 establishes the unsigned arithmetic comparison, not a
claim that application counters may silently wrap. The baseline is C17; the
inspected text is a C11 committee draft, not the unavailable published C17 text.
This core uses ordinary C99-era arrays, structures and integer operations; it
does not rely on a newer language scheduler or coroutine facility.

Prerequisite learner artifacts are the T20 explicit state-machine/component
interface, T21 failure/partial-progress contract and T29 bounded foreground/ISR
ownership argument. This exercise implements its own scheduling mechanism; it
does not require or invoke an RTOS. The later FreeRTOS and original runtime
comparisons are separate components. Supply `superloop.c` from the public stub,
not a private assessor implementation.

## Environment and enforcement boundaries

Use profile C, ISO C17 with GCC and Clang, debug/release and ASan/UBSan builds.
The mechanism is a single externally serialized owner: asynchronous callers
must marshal work through a separately justified interface. There are no locks,
callbacks, allocation or system calls in the core. Host Python, process control,
stdio and sanitizer runtimes are verification dependencies. Object-code audits
explicitly enable strong stack protection and require its `__stack_chk_fail`
runtime boundary, alongside any compiler-generated memory helpers. The first
audit's incomplete helper inventory failed on the host compiler's default stack
protection; that failed attempt is retained. This hosted campaign is not a
strict bare-metal link or a physical timing measurement.

ISO C does not prove that a caller honors quotas, keeps a control object valid,
or never overwrites live work. Diagnostics and sanitizers cover selected defects;
the public representation remains writable by a mistaken caller. Explicit tests,
finite models and programmer discipline establish different bounded claims.

## Supplied input and required implementation

Implement `superloop.h` operations with valid, disjoint, live controls and output
regions. Initialize only when no previous work or borrowed result survives.
Sources are copied on task admission. Do not use a scheduler, queue, parser or
coroutine library implementing the mechanism. Use the supplied C tests/replay
and independent Python oracle as checks after predicting results.

Ordered activities cost2,7,1 units: one original loop costs10 and the longest
activity costs7. With a resumable quota2 the representative dispatch round is
2,2,1, costing5. It does not finish all seven units of the long activity. A job
polled just after the previous poll waits up to the round duration under this
model; its own one-unit service yields response bounds11 or6. No asynchronous
interference or unbounded service is hidden in those numbers.

The concrete continuation copies up to128 bytes and converts ASCII lowercase
octets0x61–0x7a to0x41–0x5a. Other byte values are preserved; this is an explicitly
ASCII-octet operation, not a locale-aware character conversion. Each processed
byte costs one model unit. The100-byte source repeats the26 lowercase octets;
quota2 requires50 dispatches and produces the same100 output bytes as quota100.
The other two activities cost2 and1 per round, so observed logical poll gaps
are5 and103. Output ownership and cursor must survive every yield.

## Representation, invariants and ownership

`ScTask` owns copied input and private staged output, length, cursor, terminal
state and emission count. READY implies cursor<length; completion publishes DONE
and exactly one emission. Empty input is already complete with one emission.
Step after terminal returns CLOSED without changing work output. Reading a
completed result copies it and does not emit another completion. A short output
capacity is refused before any output is copied. Cancellation at any of the50
intermediate yields publishes CANCELLED, exposes no partial result and prevents
further work. Cancellation after completed yield50 is CLOSED; committed output
is not retracted. This transactional-output choice is part of the contract.

Three independent task records must be dispatched through all their slices,
including cancellation at every yield. The caller must not equate a saved
program-counter number with complete ownership state. Unlike a language coroutine,
this C representation has no automatic destruction or cancellation cleanup.

`ScEvents` is a FIFO of three copied `{id,value}` records. Push1..5 admits1,2,3
and returns FULL for4,5 while incrementing the explicit rejection count. Repeated
IDs are separate caller-supplied commands; this interface does not deduplicate.
Pop transfers a copy exactly once in FIFO order. Finite telemetry refuses RANGE
before wrapping; draining remains legal at maximum admission count.

`ScLatest` has a different contract: five writes10..50 retain only50, with four
superseded observations. It is suitable for a latest desired state, not five
independent commands. Its update counters and pending state are separate from
periodic releases and the event FIFO. Derive what is intentionally discarded.

## Periodic release and overload semantics

`ScReleases` contains a positive period, next release, last successful observation,
exact seen/lost counts and two queued release timestamps. Time is a full64-bit
monotonic model counter; rollover arithmetic is assessed separately in T30.04.
Reject backwards observations. Calculate the due count arithmetically, without
an unbounded catch-up loop. Refuse unrepresentable counts before publication.
The last representable release may be admitted once; an exhausted source stops
releasing while queued work can still drain.

COALESCE replaces queued stale work and all intermediate due releases with the
newest due release. With polls0,4,8,20 and period4 starting0, six releases have
occurred, five are lost and20 remains. A poll21 retains20. REJECT_STALE removes
queued releases older than the observation and only admits a due release exactly
at the observation; poll21 therefore removes20 and reports six lost. Repeating
the same observation never duplicates a release. Both policies expose their loss.

LOSSLESS admits the entire due set only when it fits the two slots. Otherwise
FULL preserves next, counts, queue and last-successful-observation. After draining
0 and4, a jump from the next release8 to20 would need four records and is refused;
even after separately accepting8, releases12,16,20 still exceed two slots. No
queue size can satisfy indefinitely excessive arrival. Rejection is an admission
failure requiring changed service/admission assumptions, not silent acceptance.
The test's subsequent historical horizon16 is a model replay after a refused
observation, not permission for a real clock to move backwards. An operational
caller must reject that workload or increase proven service capacity.

## Fairness, priorities and replenished budget

`ScSchedule` selects one unit-cost dispatch per strictly increasing logical tick.
Successful empty observations advance its time bookkeeping but preserve the
selection output. Invalid observations change nothing. READY masks describe up
to four slots; selection itself does not execute or preempt work.

ROUND_ROBIN rotates all eligible slots. With A/B/C continuously ready its trace
is ABCABC. PRIORITY selects continuously ready H(slot3) before the lower A/B/C
class; its trace is HHH and lower work can starve. Rotation within the lower
class does not solve that starvation. BUDGETED grants H one unit each aligned
four-tick window, with no carryover; ordinary work uses round-robin otherwise.
For all four continually ready at unit cost the trace is HABC repeated and each
ordinary service gap is4. State the exact unit-cost and readiness assumptions.
An unbounded callback or dispatch longer than a unit invalidates this bound.
This is a fixed replenishment policy, not CBS or a general reservation theorem.

## Failure, diagnosis and the nonyielding variant

`sc_bound` returns UNBOUNDED and preserves output if any required service bound
is unknown. This means no finite bound has been justified, not a claim that every
such run is infinite. Checked addition refuses arithmetic that would invalidate
the resource/latency statement. Invalid arguments do not establish recoverable
arbitrary memory corruption; valid object premises still apply.

The original `blocking.c` prints its entered marker and performs volatile reads
forever without yielding. The campaign terminates only that disposable child
after a bounded observation. Its missing return and source control flow diagnose
why cooperative scheduling cannot regain authority. That intentional observation
timeout is separately labeled; it never counts as a positive scheduler run or
as one of the semantic mutant rejections. No deployed driver is contacted.

## Deliverables and verification

Submit the representation/ownership proof, exact six objective arguments, completed
source, trace and generated-code/resource evidence. Independently predict the
fixture, burst admission IDs, release counts, cancellation boundaries, and HABC
service bound. Compare replay with every four-step ready-mask word for all three
policies:12,288 histories. Independently enumerate1,008 nondecreasing three-poll
histories over0..6 for periods1..4 and the three release policies.

The public tests require actual copied-source independence, all51 cancellation
boundaries including completed output, finite-counter cleanup, final release at
UINT64_MAX, lossless refusal, monotonic-time refusal and exact output preservation.
Seven compiled flaws must fail an ordinary SC_CHECK assertion: accepting an
unknown bound, redispatching terminal work, exposing cancelled partial output,
overwriting the full command FIFO, misclassifying lossless admission, failing
round-robin rotation, and ignoring H's replenishment budget. The unsolved public
stub must compile then fail semantically. Crashes, compiler errors and sanitizer
reports are not successful negative controls.

## Rubric and graduated hints

For `.01.a`, level2 derives10/7/5 and position/own-cost response; level3 invalidates
the bound when a driver never returns. For `.01.b`, level2 proves exactly the three
accepted command records; level3 justifies a different latest-state contract.
For `.01.c`, level2 implements resumable equivalent output and5-versus103 gaps;
level3 distinguishes strict-priority starvation from a bounded equal-class delay.
For `.02.a`, level2 owns complete continuation state and emits once; level3 handles
every cancellation point without publishing partial output. For `.02.b`, level2
implements bounded release/loss accounting; level3 rejects inadmissible no-loss
work rather than hiding missed jobs. For `.02.c`, level2 explains ABC/HHH; level3
derives HABC's bound and identifies assumptions that invalidate it.

In every criterion, level0 is a wrong/missing mechanism or result; level1 reproduces
the visible trace without its invariant. Hints: separate accepted work from latest
state; separate partial progress from public result; precompute due counts;
make the ready policy independent of the actual callback's ability to return.
Full implementations and causal solution notes remain in the assessor package.

## Changed assumptions and assessment separation

All six original delayed tasks remain mandatory at least seven days after study:
unbounded driver, latest-state replacement, high-priority starvation, cancellation
at every yield, no-drop admission and replenished H budget. Their reference support
is concrete here; author execution does not satisfy an unaided learner attempt.
Change quota, input length, clock width or event semantics and rederive the affected
arguments rather than merely changing test constants. A smaller timer requires a
separate half-range/observation-gap contract, not substitution of a narrow type.

## Security scope and validation limitations

Only original local teaching programs, synthetic bytes and a disposable child
are used. The defects violate bounded progress, ownership or admission properties;
no crash is described as code execution. No host settings, external service,
real credentials, board flashing or production equipment is involved.

Exact status is recorded by the source-bound successor campaign and guard. Finite
enumeration does not establish every input, asynchronous interleaving or physical
timing. External human review, independent delayed learner performance and any
physical observations remain separately unperformed unless explicitly recorded.
