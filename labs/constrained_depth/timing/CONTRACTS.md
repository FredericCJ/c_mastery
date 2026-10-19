# PR-T30-03: bounded chronology, coherent epochs and latency evidence

## Identity and coverage

This successor assesses `AX-PR-T30-03-T30.04.a/b/c` and
`AX-PR-T30-03-T30.05.a/b/c`, with their six original `-DELAYED` variants.
Families combine semantic derivation, construction, boundaries, diagnosis,
falsification, resource analysis and independent transfer. The retained project,
fixture and criterion text remain historical; this contract adds exact executable
support without changing their identities or hours.

## Sources and semantic boundaries

See `research/v5/expansion/legacy_constrained_timing/resources.json`: actual
N1570 C11 draft6.2.5p9 unsigned reduction and7.17.3p6 sequential consistency;
POSIX.1-2024 `clock_gettime`/`clock_getres` DESCRIPTION; and the actual original
T30 teaching/project/fixture/criterion analyses. The exact published C17 remains
unavailable. C17 is the implementation baseline; C11 introduced these atomics.
No new C23 semantic claim follows from compiler mode acceptance. Unsigned residue
arithmetic is ISO C; pthread scheduling, POSIX clocks and file synchronization are
host contracts. An atomic clock struct is an original teaching representation,
not a claim about a particular peripheral register or board counter.

## Prerequisites and supplied artifacts

Bring checked integer arithmetic, unsigned promotion rules, copied-result failure
contracts, atomic operations, and the original PR-T30-03 fixture prediction.
Implement the supplied unsolved `timing.c` against `timing.h`. Supplied tests,
complete comparison replay, Python oracle, native concurrency, measurement and
separate-process persistence drivers are learner-safe verification artifacts.
The assessor implementation and causal analysis remain separately packaged.
No timer, scheduling or statistics library implements the studied core.

## Environment and dependency profile

C17 hosted profileC, 8-bit bytes and exact uint32_t/uint64_t, GCC/Clang debug,
optimized and ASan+UBSan; native concurrency additionally runs both available
ThreadSanitizer compilers. pthreads and clock/file calls belong only to the
host drivers. Atomic operations may use implementation runtime support; neither
lock freedom nor a freestanding link is promised. Inputs, outputs and clock
control are live/disjoint. Public fields must not be forged; initialization is
exclusive before any thread can use the clock. No signal-handler safety follows.

## Concrete problem and input/output contracts

Implement positive-resolution timeout conversion with rounding upward, supported
counter widths exactly8 or16, and a requested delay strictly below half range.
1.5ms at1ms becomes2ticks; at250µs it becomes6. A rounded128-tick8-bit request
is refused unchanged. Use division/remainder; do not add resolution-minus-one
to a near-MAX duration. Zero duration is valid; zero resolution is invalid.

`ct_order(bits,now,deadline)` returns EQUAL, DUE or FUTURE only under an externally
established true separation strictly less than half range. Separation exactly
half range returns AMBIGUOUS with output unchanged. Other residues cannot reveal
a caller's violated history assumption: observed delta200 in8bits can describe
a past interval200 or a future separation56. Do not claim that a two-sample API
detects every missed wrap. Exhaust all65536 eight-bit pairs against lifted integer
candidates; test49 selected16-bit boundary pairs. The horizon extends to32768
ticks (8.192s at250µs), exclusive; it never becomes unbounded.

Elapsed250→4 is10 only under the separate premise of less than one full wrap.
Actual durations10 and266 produce identical residues. A supplied false bound
flag returns AMBIGUOUS; a true flag records a caller premise, not runtime proof.

## Coherent epoch representation and reset

`CtStamp` contains positive boot identity, epoch, low counter, bit width and
positive nanoseconds-per-tick. Its lifted count is epoch*modulus+low, checked
before multiplication/addition. `ct_duration` requires matching boot, width and
resolution, ordered lifted counters and representable nanoseconds. It supports
long intervals such as255→772 (517ticks) without guessing wrap history.

`CtClock` holds all fields and a sequence as atomic objects. Every access uses
sequential consistency. A writer snapshots and validates first, then a successful
CAS changes even sequence to odd, publishes the fields, and advances to the next
even sequence. Same-boot time cannot go backward or change representation;
new boot identity must be greater. Competing or exhausted admission returns
BUSY/RANGE without publication. Sequence never wraps: MAX-1 is a readable final
even state; later writes refuse. Initialization requires whole-world quiescence.

A reader accepts only equal even sequence values around all field loads. Bounded
retry exhaustion preserves output. The synchronous diagnostic hook may perform
a valid publication after epoch or after all fields; it must terminate and may
not destroy the objects. Force epoch0/low255→epoch1/low0 while reading: a defined
but unvalidated pair can be0; the validated snapshot retries and returns256.
Atomic individual loads alone do not make the combined value atomic. The finite
SC model enumerates3432 operation schedules; omitting the final validation admits
mixed states. This model is not a weak-order model checker or proof of all C code.

Encode a canonical40-byte little-endian record: magic CTS1, version1, boot8,
epoch8, low4, bits4 and quantum8 after the8-byte magic/version header. Decoder
requires exact extent, magic/version and a valid representable stamp; it stages
output. It provides neither authentication nor corruption-proof storage. Write
it to an exclusive temporary file, synchronize it, then start a new process that
reads boot7 and compares with declared reset boot8. Chronology is DIFFERENT and
output remains99 even if the new numeric count is larger. A trusted monotonically
allocated boot identity is an explicit system prerequisite; this example does
not implement crash-safe boot-counter allocation or prove physical reset continuity.

## Metric and distribution invariants

The supplied release10/start12/finish15 trace has latency2, elapsed execution3,
response5. Require release≤start≤finish. Up to64 nonempty half-open slices exactly
partition[start,finish), each classified CPU, ISR, PREEMPTED, DMA or BLOCKED.
Reject gaps, overlap, backwards slices, unknown kinds or uncovered finish without
publishing results. Each interval belongs to one category; response equals
latency plus wall interval, while wall interval is the sum of all five categories.

For CPU3 plus two1-tick ISR slices and queue delay2, response is7. Do not add ISR
cost to a wall interval that already contains it. A2-tick preemption similarly
keeps CPU3 but expands the interval to5. Add one DMA-stall slice without an ISR:
response becomes8 with CPU3/ISR2/DMA1. These are declared synthetic traces; they
do not measure DMA or prove a peripheral contention model.

Release jitter is a signed magnitude relative to an ideal reference: ideals0,4,8
and releases0,5,9 give0,1,1. This avoids an implementation-defined large unsigned
to signed conversion. Statistics over1..128 samples return exact checked sum,
min/max, quotient/remainder mean, and contiguous runs strictly above a threshold.
For[2,2,3,9], n4, mean4, min2, max9. Reordering five2s and three9s preserves the
histogram/sum37 but changes longest exceedance run1→3 and run count3→1. A finite
observed maximum does not bound future latency; correlated bursts invalidate
unjustified independent-sample tail arguments.

## Experimental deliverables and resource costs

Submit derivations, representation and reset authority, original fixture results,
counterexamples, complete replay evidence, coherent snapshot protocol, metrics
ledger and raw measured samples. The host experiment records clock_getres for
CLOCK_MONOTONIC and CLOCK_THREAD_CPUTIME_ID, plus24 dispersed and24 clustered
pause samples with identical bounded arithmetic work and six explicit1ms sleeps
per pattern. Separate wall and thread CPU measurements; clock reads themselves
have cost and the measurements are not simultaneous. Do not enforce a fabricated
speed ratio or claim sleeps are physical DMA/preemption observations.

Replay actual raw values through independent Python statistics. Record compiler,
optimization, sanitizers, host clock resolution, sample order and injected pauses.
The native concurrency driver publishes10000 stamps and performs40000 bounded
read attempts plus a final joined read; it checks every successful snapshot and
unchanged BUSY outputs. It does not assume readers have a scheduling latency bound.
Record actual object sizes, local compiler stack use and generated helper imports.
Fixed integer operations are O(1); snapshots O(attempt limit), metric/statistics
work O(slices/samples), codec40bytes. No allocation occurs in the core; host thread
and stdio allocations remain declared harness dependencies.

## Acceptance and falsification

Pass every ordinary profile and all expected semantic controls. Controls omit
rounding, admit exact half range, skip snapshot validation, accept different boot,
double-count ISR, misclassify DMA, and erase temporal run structure. Each must
compile then fail a concrete assertion, separately from the unsolved stub.
Crashes, sanitizer errors, compilation failures and harness timeouts are not
successful semantic controls. Check complete pair count, counterexamples,
native results, the independent SC event model, actual two-process reset record,
source identity and raw clock observations. Keep physical claims separate.

## Objective rubric

- **T30.04.a:** level2 derives rounding, units and admitted horizon and demonstrates
  torn acquisition; level3 executes16-bit250µs conversion and revisits resolution,
  counter acquisition and horizon separately.
- **T30.04.b:** level2 proves unsigned due/future/equal/ambiguous under the actual
  premise and exhausts65536 pairs; level3 adds coherent epochs and establishes
  publication/finite-sequence invariants, not merely a wider typedef.
- **T30.04.c:** level2 exhibits10/266 ambiguity and the distinct elapsed premise;
  level3 persists explicit boot metadata and rejects cross-reset chronology,
  identifying the missing boot-identity authority rather than assuming continuity.
- **T30.05.a:** level2 reports latency/elapsed/response and referenced jitter;
  level3 adds preemption and keeps CPU demand separate from elapsed interval.
- **T30.05.b:** level2 partitions CPU/ISR/queue costs without double counting;
  level3 models DMA stalls as a separate category and rejects overlap or omissions.
- **T30.05.c:** level2 gives exact sample statistics and a reproducible actual
  target-campaign design; level3 executes correlated pauses, preserves order and
  limits tail/WCET conclusions instead of treating repeated samples as a proof.

Level0 lacks correct evidence; level1 repeats the fixture without resolving its
misconception. Level2 meets the stated contract with independent falsification.
Level3 requires the delayed changed-condition argument and implementation; author
reference execution is not a learner assessment or absolute mastery certificate.

## Hints and independent transfer

Hint1: list all integer lifts of a pair before choosing a chronology. Hint2:
separate residue width, duration quantum, epoch acquisition and boot identity.
Hint3: partition the wall interval before adding any interference. The six exact
original delayed variants retain their minimum-seven-day unaided requirement.
Do not distribute assessor source as the worked learner solution.

## Isolation, status and limits

Only original local programs, synthetic counters and exclusive temporary files
are used. No host clock changes, security changes, external targets or flashing.
Current author results belong to the source-pinned successor report. Physical
target timing, independently reviewed proofs and delayed learner execution remain
unperformed. A larger counter, a successful sanitizer run or an observed maximum
does not remove the declared chronology, publication or measurement limits.
