# T52.07 admission, overload and visible loss

## 1. Identity and exercise families

This required continuation implements `AX-PR-T52-07-T52.07.a`, `.b`, `.c`
and their original seven-day delayed variants. Families A/B/C/D/F/G/H/I/J
assess distinct admission and queue bounds, construction, finite horizons,
semantic controls, independent ledgers, overload recovery, resource accounting,
changed arrival assumptions and actual synchronization composition. The original
project, fixture, unit identities, hours and acceptance receipts remain intact.

## 2. Objectives and inspected references

Use `R-LOVERLOAD-DL` in
`research/v5/expansion/legacy_realtime_overload/resources.json`. The actual
Linux documentation sections3.2,3.4 and4 distinguish feasibility assumptions,
application parameters and admission-time allocation. They do not specify this
course's queue policies or prove this controller. The fixed-slot cyclic argument
below is original course analysis. The actual `cmx_cancel`, semaphore and mutex
implementation was inspected for the bounded backpressure experiment; it is a
course producer, not an ISO C guarantee or a current Linux implementation.

## 3. Prerequisites and learner artifacts

Bring the completed `PR-T52-01`/`PR-T52-02` job identity and schedule ledgers,
`PR-T52-05`/`PR-T52-06` budget and overload distinctions, `PR-T51-01` mutex
and `PR-T51-03` semaphore artifacts. Import an explicit completed CMX source
and `cmx.h`; a producer stub must fail the actual dependency-cycle test. Keep
the original capacity2 fixture and predict its rejected ID before execution.

## 4. Environment, language and dependencies

Use C17, externally serialized operations and caller-owned, live, disjoint
storage. The model has no heap, threads, interrupt handler or preexisting
scheduler library. Host Python integers, stdio and sanitizers are verification
tools. The strict Cortex-M4 image links only the explicit implementation, CMX,
course byte helpers and public test/startup translation unit. Audit symbols,
map, disassembly, stack files and size; no implicit libc/libgcc/libatomic or
division library is permitted. Logical ticks and emulator execution do not
establish physical worst-case time. No Linux scheduling configuration is changed.

## 5. Problem, concrete inputs and operations

Implement `overload.h` using the unsolved public `overload.c`.

`od_admit` stages a candidate in a set of at most4 tasks. Each task has
positive integer cost C, period T≤64, implicit relative deadline T, phase0
and a criticality bit. Tasks are preemptible at tick boundaries, independent,
and release exactly every T. A supervisor occupies slot t divisible by4;
other slots select EDF with stable task-index ties. Compute the joint cycle
LCM(4,T0,...), simulate only up to the smaller of that cycle and the supplied
horizon1..128, and check misses at the final boundary. A full cycle with no
miss returns ADMITTED and atomically installs the candidate. A witnessed miss
returns REJECTED. A shorter successful prefix returns INCONCLUSIVE and does
not install anything. INVALID and RESOURCE leave both set and decision unchanged.
This API does not admit arbitrary sporadic arrivals or jobs using average costs.

`od_run` accepts a valid task description and up to96 immutable, unique request
IDs. Each request names its stream, release, actual cost0..256, absolute deadline
and an externally supplied `io_until` eligibility time. IDs may not equal
`OD_NONE`. Deadlines are at least release and no greater than256; releases are
at most128 and I/O eligibility at most256. The simulation horizon is1..128.
The queue holds1..8 pending requests. Task budgets are charged to actual service;
an unfinished request reaching its declared cost is terminated as OVERRUN.
This does not retroactively prove that an adversarial arrival sequence meets
the periodic admission premises.

Choose REJECT_NEW, DROP_OLDEST or LATEST. Oldest means release then input index;
LATEST replaces an unfinished request of the same stream before capacity
checking. Discarding a partially executed request retains its consumed service
and records DROP with its own ID. It never creates a completed result for that
ID. These are copied synthetic requests: there is no borrowed asynchronous I/O
buffer or external callback to cancel. Real I/O cancellation needs a separate
quiescence protocol and cannot simply free a live borrowed buffer.

Within each tick, apply the optional criticality switch, expire pending deadlines,
attempt requested recovery, admit arrivals, then run one supervisor or service
slot. The switch discards pending low-criticality requests with MODE_DROP and
rejects later low requests with MODE_REJECT. High requests retain IDs, deadlines,
consumed work and budgets. This is a declared shedding policy, not a mixed-
criticality schedulability theorem. Recovery succeeds only at an empty live
queue boundary; otherwise it records refusal. It does not reset old result IDs,
revive dropped work or prove that future arrival/cost premises have recovered.

Use supervisor period4 for the composed controller, or0 only for the separate
one-service-per-tick queue comparison. Ready work selects EDF with release/input
ties. `io_until` affects readiness but still occupies queue capacity. It is a
finite self-suspension model, not an actual device driver. At the final horizon,
expire due deadlines but admit no new arrivals. Unfinished requests remain
PENDING; a harness reaching its horizon does not turn them into completions.
Every malformed input preserves the whole output. Report every request state,
executed service and finish time plus each tick's selected ID, queue occupancy,
mode and supervisor activity.

`od_backpressure` composes actual CMX operations. Start with a capacity2 queue
represented by zero free-space semaphore tokens and a producer holding the
consumer's mutex. The producer waits for space; the consumer waits for that
mutex. Capture both wait states and the owner. Repair by cancelling the producer's
wait, releasing its mutex, then letting the consumer release space. Preserve the
producer's terminal `CMX_CLOSED` result; this retained API has no distinct
cancellation result code. Drain and close all local synchronization registrations
before returning the value-only trace. No timeout or priority donation can
remove the circular resource dependency without a changed protocol.

## 6. Mechanisms and misconceptions

For three arrivals per tick during four ticks, capacity2, cost1 and one service
per tick, derive four completions, seven rejections and one pending item. The
accepted IDs are0,1,3,6,9; ID9 remains pending. Feasible periodic CPU demand does
not prevent this queue overload. Latest-value replacement changes which request
can receive a response, not only queue performance.

With tasks C1/T4 and C2/T4 plus the reserved supervisor slot, a full four-tick
cycle admits both. Adding another C1/T4 must reject atomically. C1/T7 with an
eight-tick horizon is inconclusive; a complete28-tick cycle can establish the
declared cyclic result. A finite prefix is not a global acceptance certificate.

A request costing8 on a stream budget2 must consume exactly2 then terminate as
OVERRUN, allowing ordinary requests and the supervisor to continue. Two blocked
I/O requests can fill the queue while consuming no CPU. The supervisor still
runs, but neither empty queues nor application response follows from CPU
admission. Expose this difference with exact IDs and pending/expired states.

## 7. Required reasoning and invariants

Prove the admission staging rule, checked representation bounds and cyclic
recurrence. LCM(4,T0,...,T3) is bounded by4×64^4, below2^27. At a successful
cycle boundary every implicit-deadline job is complete and the next releases,
supervisor phase and stable tie rule repeat, establishing an induction for
this exact deterministic model. A changed phase, jitter, blocking, overhead,
cost, multicore policy or scheduling quantum invalidates that argument.

Every released request has exactly one live or terminal disposition. Executed
service survives drops; only DONE represents a complete result. Peak occupancy
cannot exceed capacity. Supervisor slots occur independently of queue readiness,
so there is one heartbeat per four modeled ticks even under sustained arrivals.
This assumes the dispatcher itself executes: arbitrary unmasked latency or a
stalled physical CPU is outside the model. State who owns a mutex and free-space
token at each dependency transition; notification is not release of ownership.

Admission uses O(horizon×tasks) work and bounded task arrays. Runtime scans at
most96 records per tick and may scan them per arrival, giving bounded
O(horizon×requests²) work and fixed result storage. Separate ledger storage from
the capacity-bounded live queue and distinguish model cost from an actual port's
stack, scheduling overhead and physical response bounds.

## 8. Required deliverables

Submit implementation, admission proof and inconclusive witness, complete
ID/disposition ledgers, budget/heartbeat traces, criticality and recovery rules,
actual CMX dependency cycle/cleanup trace, and blocked-I/O counterexample. Include
REJECT_NEW versus DROP_OLDEST versus LATEST under identical inputs, the actual
completed producer identity, independent oracle comparisons, seven semantic
control diagnoses, compiler/sanitizer profiles and strict target dependency audit.

## 9. Verification and acceptance

Run admission, queue, policy, overrun, mode, io, backpressure and guards.
`oracle.py` independently generates320 admission sets and480 workloads, including
out-of-order arrivals, zero cost, delayed I/O, expired deadlines, mode switches,
full queues and recovery refusal. Its exact25895 expected rows include763
admission decisions,8812 request results and15360 tick records. Compare actual
C output row by row. Do not substitute an expected summary count for execution.

Reject seven semantic controls: ignoring capacity, silently relabeling a drop as
completion, starving the supervisor, ignoring overrun budget, failing to release
the dependency-cycle mutex, partially installing a rejected candidate, and
retaining low work across the mode switch. Own-source and actual producer stubs
must compile and fail intended assertions. Compiler failures, arbitrary crashes,
sanitizer diagnostics or a generic target fault do not count as successful
negative tests. Exact commands/results are produced by `tools/run_overload_depth.py`.

## 10. Objective-specific rubric

For `.a`, level0 equates CPU admission with queue capacity; level1 reports only
aggregate throughput; level2 derives every accepted/rejected/pending ID and policy
change; level3 explains the actual mutex/free-space wait cycle and repair ownership.
For `.b`, level0 installs an inconclusive candidate; level1 handles ordinary
periodic load; level2 proves atomic admission and supervisor progress under stated
premises; level3 accounts for every preserved/dropped resource during mode change.
For `.c`, level0 treats continuing execution as recovery; level1 measures only
heartbeats; level2 provides all terminal outcomes, bounds and controls; level3
distinguishes blocked-I/O memory pressure from CPU demand and states the newly
required I/O completion/quiescence assumptions.

## 11. Hints and separate assessor analysis

Hint1: a ledger entry is not the same resource as a live queue slot. Hint2: inspect
the final simulated boundary before claiming a complete cycle. Hint3: record work
already consumed before dropping a job. Hint4: draw the two wait edges while the
producer owns the mutex. The private reference and causal analysis are
`assessor/legacy_realtime/overload.c` and `overload.md`; public implementation
and imported producer stubs remain unsolved.

## 12. Changed-condition and delayed variants

Keep the original seven-day delay. `.a-DELAYED` requires the actual CMX mutex/
free-space dependency experiment and cancellation/handoff repair. `.b-DELAYED`
switches at tick2 with low I/O-blocked and high runnable work, retains high
progress, rejects later low IDs and compares recovery at2 versus6. `.c-DELAYED`
uses two requests blocked until100: at horizon8 both remain pending and later
requests are rejected; at deadline20 they expire explicitly. These are implemented
preparation variants, not completed independent learner attempts.

## 13. Security and containment

Only original local teaching programs, synthetic request IDs and owned emulator
images are in scope. An adversarial producer controls release rate, actual cost
and finite I/O eligibility to exhaust a teaching queue or delay synthetic work.
Harmless success conditions are exact rejected IDs, a violated synthetic deadline
or the named control's accounting failure. Repair the queue, budget or dependency
policy and preserve regression traces. No real service, physical device, host
scheduler/security setting or external data is touched; resource exhaustion in
this model is not evidence of code execution or privilege escalation.

## 14. Validation status and limitations

Authored code and passing pilot observations are distinct from source-bound full
campaigns. The current report and evidence guards identify exact observed profiles
and controls. No hardware timing, independent human review, learner acceptance,
arbitrary sporadic admission, actual I/O cancellation or physical runtime port is
inferred from this finite model. Finite passing tests do not prove absolute mastery.
